#include "MemoryFileETagService.h"

#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"

namespace FileETagService
{

MemoryFileETagService::MemoryFileETagService()
{
    const auto& conf = ConfigReader::GetInstance();
    const auto& data_path = conf.GetETagData();
    const std::string data_path_str = utils::path::to_string(data_path);

    if (std::filesystem::exists(data_path))
    {
        std::ifstream ifs{data_path, std::ios::in};
        if (!ifs.is_open())
        {
            LOG_ERROR_FMT("Unable to open '{}', unable to recover file etags.", data_path_str)
            return;
        }

        // line string like this: path@etag
        std::string raw_line{};
        while (std::getline(ifs, raw_line))
        {
            const std::string_view line = raw_line;
            auto pos = line.find_last_of('@');
            if (pos == std::string_view::npos)
            {
                continue;
            }

            auto path_str = line.substr(0, pos);
            auto etag = line.substr(pos + 1);
            if (path_str.empty() || etag.empty())
            {
                continue;
            }

            etag_map_.insert({ETagMapKeyT{path_str}, ETagMapValueT{etag}});
        }
    }

    data_.open(data_path, std::ios::out);
    if (!data_.is_open())
    {
        LOG_ERROR_FMT("Unable to save file etag to '{}', file properties are lost when the server stops.",
                      data_path_str)
    }
}

MemoryFileETagService::~MemoryFileETagService()
{
    if (!data_.is_open())
        return;

    for (const auto& [path_sha, etag] : etag_map_)
    {
        data_ << utils::path::to_string(path_sha) << ',' << etag << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    LOG_INFO_FMT("The file etag have been saved to {}", utils::path::to_string(conf.GetETagData()))
}

std::string MemoryFileETagService::Get(const std::filesystem::path& path) noexcept
{
    const auto& it = etag_map_.find(path);
    if (it == etag_map_.end())
    {
        return {""};
    }

    return {it->second};
}

std::string MemoryFileETagService::Set(const std::filesystem::path& path) noexcept
{
    try
    {
        const auto& it = etag_map_.find(path);
        if (it != etag_map_.end())
        {
            etag_map_.erase(it);
        }

        ETagMapValueT sha{};
        if (std::filesystem::is_directory(path))
        {
            sha = utils::sha256(utils::path::to_string(path));
        }
        else if (std::filesystem::is_regular_file(path))
        {
            sha = utils::sha256(path);
        }
        else
        {
            LOG_WARN("Unexpected file type.")
            return {""};
        }

        etag_map_.insert({path, sha});
        return sha;
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        return {""};
    }
}

} // namespace FileETagService
