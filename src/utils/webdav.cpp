#include "webdav.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <format>
#include <mutex>
#include <regex>
#include <stack>
#include <string>
#include <utility>

#include <PicoSHA2/picosha2.h>

#include "http_exceptions.hpp"
#include "path.h"
#include "services/FileETagServiceFactory.h"
#include "services/FileLockService.h"
#include "utils/path.h"

std::mutex utils_webdav_ComputeEtag_LOCK;

namespace utils::webdav
{

pugi::xml_node generate_multistatus_header(pugi::xml_node& xml_doc)
{
    pugi::xml_node xml_ms = xml_doc.append_child("D:multistatus");
    xml_ms.append_attribute("xmlns:D").set_value("DAV:");

    return xml_ms;
}

void generate_response(pugi::xml_node& multistatus, const std::filesystem::path& path)
{
    bool is_directory = std::filesystem::is_directory(path);
    auto xml_res = multistatus.append_child("D:response");

    auto relative_file_path = std::filesystem::relative(path, std::filesystem::current_path());
    std::string file_path = utils::path::with_separator(relative_file_path, 1, is_directory, '/');
    file_path.insert(file_path.begin(), '/'); // begin with '/'

    xml_res.append_child("D:href").text().set(file_path.c_str());

    auto xml_propstat = xml_res.append_child("D:propstat");
    auto xml_prop = xml_propstat.append_child("D:prop");

    if (is_directory)
    {
        xml_prop.append_child("D:resourcetype").append_child("D:collection");
    }
    else
    {
        xml_prop.append_child("D:getcontentlength").text().set(std::to_string(std::filesystem::file_size(path)).c_str());

        auto file_time = std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(path));
        std::time_t file_time_t = std::chrono::system_clock::to_time_t(file_time);
        char buffer[100];
        std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&file_time_t));

        xml_prop.append_child("D:getlastmodified").text().set(buffer);
    }

    xml_propstat.append_child("D:status").text().set("HTTP/1.1 200 OK");
}

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        generate_response(multistatus, entry.path());
    }
}

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth)
{
    namespace fs = std::filesystem;

    if (depth <= 0 || dirs.empty())
        return;

    std::stack<fs::path> sub_dirs;

    while (!dirs.empty())
    {
        for (const auto& entry : fs::directory_iterator(dirs.top()))
        {
            auto xml_res = multistatus.append_child("D:response");
            std::string file_path = utils::path::with_separator(fs::relative(entry, std::filesystem::current_path()), 1, entry.is_directory(), '/');
            file_path.insert(file_path.begin(), '/');
            xml_res.append_child("D:href").set_value(file_path.c_str());

            auto xml_propstat = xml_res.append_child("D:propstat");
            auto xml_prop = xml_propstat.append_child("D:prop");

            if (entry.is_directory())
            {
                sub_dirs.push(entry.path());
                xml_prop.append_child("D:resourcetype").append_child("D:collection");
            }
            else if (entry.is_regular_file())
            {
                xml_prop.append_child("D:getcontentlength").text().set(std::to_string(entry.file_size()).c_str());
                auto file_time = std::chrono::clock_cast<std::chrono::system_clock>(entry.last_write_time());
                std::time_t file_time_t = std::chrono::system_clock::to_time_t(file_time);
                char buffer[100];
                std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&file_time_t));
                xml_prop.append_child("D:getlastmodified").text().set(buffer);
            }

            xml_propstat.append_child("D:status").text().set("HTTP/1.1 200 OK");
        }
        dirs.pop();
    }

    generate_response_list_recurse(multistatus, std::move(sub_dirs), depth - 1);
}

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth)
{
    std::stack<std::filesystem::path> stack;
    stack.push(path);
    generate_response_list_recurse(multistatus, std::move(stack), depth);
}

void check_precondition(const std::filesystem::path& abs_path, std::string conditions)
{
    static auto& lock_service = FileLock::Service::GetInstance();
    static auto& etag_service = FileETagService::GetService();

    // <resource-tag> (condition1) (condition2) ...
    static const std::regex extract_resource_tag{R"(^\<(/[^\s]+/?)\> (.*)+)"};

    /*
        (<urn:uuid:lock-token>) -> <urn:uuid:lock-token>
        ([sha256]) -> [sha256]
        (Not <urn:uuid:lock-token>) -> Not <urn:uuid:lock-token>
        (Not [sha256]) -> Not [sha256]
        (Not <urn:uuid:lock-token> [sha256]) -> Not <urn:uuid:lock-token> [sha256]
        (<urn:uuid:lock-token> Not [sha256]) -> <urn:uuid:lock-token> Not [sha256]
        (Not <urn:uuid:lock-token> Not [sha256] -> Not <urn:uuid:lock-token> Not [sha256]
    */
    static const std::regex extract_condition{R"((Not )?(\<urn:uuid:[^\s]+\>|\[[A-Za-z0-9]{64}\]))"};

    // [sha256]
    static const std::regex is_entity_tag{R"(^\[(.*)\]$)"};

    // <urn:uuid:lock-token>
    static const std::regex is_state_token{R"(\<(.*)\>)"};

    //-- try to extract Resource-Tag --//
    // to which resource should the conditions be applied
    std::string resource_path = abs_path.string();
    {
        std::smatch matched_result;
        if (std::regex_search(conditions, matched_result, extract_resource_tag))
        {
            resource_path = matched_result[1].str();
            conditions = matched_result[2].str();
        }
    }

    //-- try to parse No-tag-list --//
    std::sregex_iterator it{conditions.begin(), conditions.end(), extract_condition};
    std::sregex_iterator end;
    bool ok = false;

    // check preconditions
    while (it != end)
    {
        std::smatch mres = *it++;
        bool not_flag = !(mres[1].str().empty());
        std::string condition = mres[2].str();

        // lock token
        if (condition.starts_with('<') && condition.ends_with('>'))
        {
            bool res = lock_service.LockedByToken(resource_path, condition);
            if (not_flag)
                res = !res;
            if (!res)
                throw PreconditionFailedException("precondition failed");
            ok = true;
        }

        if (condition.starts_with('[') && condition.ends_with(']'))
        {
            bool res = etag_service.Get(resource_path) == std::format("[{}]", condition);
            if (not_flag)
                res = !res;
            if (!res)
                throw PreconditionFailedException("precondition failed");
            ok = true;
        }
    }

    if (!ok)
    {
        throw PreconditionFailedException("precondition failed");
    }
}

} // namespace utils::webdav
