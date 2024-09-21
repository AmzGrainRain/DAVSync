#include "MemoryFileETagService.h"

#include <exception>
#include <filesystem>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

#include "ConfigReader.h"
#include "utils.h"
#include "utils/path.h"

namespace FileETagService
{

MemoryFileETagService::MemoryFileETagService()
{
    const auto& conf = ConfigReader::GetInstance();

    if (std::filesystem::exists(conf.GetETagData()))
    {
        std::ifstream ifs{conf.GetETagData(), std::ios::in};
        if (!ifs.is_open())
        {
            spdlog::warn("Unable to open '{}', unable to recover file etags.", conf.GetETagData().string());
            return;
        }

        // line string like this: path_sha,etag
        std::string raw_line{};
        while (std::getline(ifs, raw_line))
        {
            std::string_view line = raw_line;
            auto pos = line.find_last_of(',');
            if (pos == std::string_view::npos)
            {
                continue;
            }

            auto path_sha = line.substr(0, pos);
            auto etag = line.substr(pos + 1);
            if (path_sha.empty() || etag.empty())
            {
                continue;
            }

            etag_map_.insert({std::string{path_sha}, std::string{etag}});
        }
    }

    data_.open(conf.GetETagData(), std::ios::out);
    if (!data_.is_open())
    {
        spdlog::warn("Unable to save file etag to '{}', file properties are lost when the server stops.",
                     conf.GetETagData().string());
    }
}

MemoryFileETagService::~MemoryFileETagService()
{
    if (!data_.is_open())
        return;

    for (const auto& [path_sha, etag] : etag_map_)
    {
        data_ << path_sha << ',' << etag << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    spdlog::info("The file etag have been saved to {}", conf.GetETagData().string());
}

std::string MemoryFileETagService::Get(const std::string& path_sha)
{
    if (!etag_map_.contains(path_sha))
    {
        return {""};
    }

    return {etag_map_.at(path_sha)};
}

bool MemoryFileETagService::Set(const std::filesystem::path& file)
{
    try
    {
        std::string path_str = utils::sha256(utils::path::to_string(file));
        std::string file_sha = utils::sha256(file);
        etag_map_.insert({std::move(path_str), std::move(file_sha)});
        return true;
    }
    catch (const std::exception& err)
    {
        spdlog::warn(err);
        return false;
    }
}

} // namespace FileETagService
