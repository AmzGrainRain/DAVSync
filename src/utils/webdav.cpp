#include "webdav.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <format>
#include <mutex>
#include <stack>
#include <utility>

#include "path.h"
#include "utils/path.h"
#include <PicoSHA2/picosha2.h>

std::mutex utils_webdav_ComputeEtag_LOCK;

namespace utils::webdav
{

pugi::xml_node generate_multistatus(pugi::xml_node& xml_doc, bool ssl_enabled, const std::string& host)
{
    const std::string my = std::format("{}://{}", ssl_enabled ? "https" : "http", host);

    pugi::xml_node xml_ms = xml_doc.append_child("D:multistatus");
    xml_ms.append_attribute("xmlns:D") = "DAV:";
    xml_ms.append_attribute("xmlns:my") = my.c_str();

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
        xml_prop.append_child("D:getcontentlength")
            .text()
            .set(std::to_string(std::filesystem::file_size(path)).c_str());

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
            std::string file_path =
                utils::path::with_separator(fs::relative(entry, std::filesystem::current_path()), 1, entry.is_directory(), '/');
            file_path.insert(file_path.begin(), '/');
            xml_res.append_child("D:href").text().set(file_path.c_str());

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

auto uri_to_absolute(const std::filesystem::path& webdav_abslute_data_path, const std::string& webdav_prefix,
                     const std::string_view& uri) -> std::filesystem::path
{
    std::string uri_str{uri.substr(webdav_prefix.size())};
    uri_str.insert(uri_str.begin(), '/');
    uri_str.insert(uri_str.begin(), '.');

    return (webdav_abslute_data_path / uri_str).lexically_normal();
}

std::optional<std::string> compute_etag(const std::filesystem::path& file_path)
{
    std::lock_guard<std::mutex> LOCK(utils_webdav_ComputeEtag_LOCK);

    std::ifstream ifs(file_path, std::ios::binary);
    if (!ifs.is_open())
    {
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(picosha2::k_digest_size);
    picosha2::hash256(ifs, buffer.begin(), buffer.end());
    return picosha2::bytes_to_hex_string(buffer.begin(), buffer.end());
}

} // namespace utils::webdav
