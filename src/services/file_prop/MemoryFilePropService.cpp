#include "MemoryFilePropService.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "ConfigReader.h"
#include "FilePropService.h"
#include "utils/string.h"

namespace FilePropService
{

MemoryFilePropService::MemoryFilePropService()
{
    const auto& conf = ConfigReader::GetInstance();

    if (std::filesystem::exists(conf.GetPropData()))
    {
        std::ifstream ifs{conf.GetPropData(), std::ios::in};
        if (!ifs.is_open())
        {
            spdlog::warn("Unable to open '{}', unable to recover file properties.", conf.GetPropData().string());
            return;
        }

        // line string like this: path_sha@mark1=value1,mark2=value2
        std::string raw_line{};
        while (std::getline(ifs, raw_line))
        {
            std::string_view line = raw_line;

            size_t pos = line.find_last_of('@');
            if (pos == std::string_view::npos)
            {
                continue;
            }

            auto path_sha = line.substr(0, pos);
            auto prop_list_str = line.substr(pos + 1);
            if (prop_list_str.empty())
            {
                prop_map_.insert({std::string{path_sha}, {}});
                continue;
            }

            ETagContainerItem props{};
            size_t start = 0;
            while (start < prop_list_str.length())
            {
                size_t end = prop_list_str.find_first_of(',');
                if (end == std::string_view::npos)
                {
                    auto pair = utils::string::split2pair(prop_list_str, '=');
                    if (!pair.first.empty())
                    {
                        props.insert(std::move(pair));
                    }

                    break;
                }

                auto prop_str = prop_list_str.substr(start, end);
                auto pair = utils::string::split2pair(prop_str, '=');
                if (!pair.first.empty())
                {
                    props.insert(std::move(pair));
                }

                start = end + 1;
            }

            prop_map_.insert({std::string{path_sha}, std::move(props)});
        }
    }

    data_.open(conf.GetPropData(), std::ios::out);
    if (!data_.is_open())
    {
        spdlog::warn("Unable to save file properties to '{}', file properties are lost when the server stops.",
                     conf.GetPropData().string());
    }
}

MemoryFilePropService::~MemoryFilePropService()
{

    if (!data_.is_open())
    {
        return;
    }

    for (const auto& [path_sha, props] : prop_map_)
    {
        data_ << path_sha << '@';

        for (const auto& [key, value] : props)
        {
            data_ << key << '=' << value << ',';
        }

        if (!props.empty())
        {
            data_.seekp(-1, std::ios::cur);
        }

        data_ << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    spdlog::info("The file attributes have been saved to {}", conf.GetPropData().string());
}

bool MemoryFilePropService::Set(const std::string& path_sha, const PropT& prop)
{
    auto it = prop_map_.find(path_sha);
    if (it == prop_map_.end())
    {
        return false;
    }

    it->second.insert({prop.first, prop.second});
    return true;
}

std::string MemoryFilePropService::Get(const std::string& path_sha, const std::string& key)
{
    auto it = prop_map_.find(path_sha);
    if (it == prop_map_.end())
    {
        return {""};
    }

    const auto& props = it->second;
    if (!props.contains(key))
    {
        return {""};
    }

    return {props.at(key)};
}

std::vector<PropT> MemoryFilePropService::GetAll(const std::string& path_sha)
{
    auto it = prop_map_.find(path_sha);
    if (it == prop_map_.end())
    {
        return {};
    }

    std::vector<PropT> props{};
    for (const auto& pair : it->second)
    {
        props.push_back(pair);
    }

    return props;
}

bool MemoryFilePropService::Remove(const std::string& path_sha, const std::string& key)
{
    auto it = prop_map_.find(path_sha);
    if (it == prop_map_.end())
    {
        return false;
    }

    auto& props = it->second;
    if (!props.contains(key))
    {
        return false;
    }

    return static_cast<size_t>(props.erase(key)) == 1;
}

bool MemoryFilePropService::RemoveAll(const std::string& path_sha)
{
    if (prop_map_.contains(path_sha))
    {
        return false;
    }

    return static_cast<size_t>(prop_map_.erase(path_sha)) == 1;
}

} // namespace FilePropService
