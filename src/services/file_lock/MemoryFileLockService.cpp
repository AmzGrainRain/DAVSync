#include "MemoryFileLockService.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ConfigReader.h"
#include "FileLockService.h"
#include "logger.hpp"
#include "utils/path.h"
#include "utils/string.h"

namespace FileLockService
{

static inline FileLockType parse_file_lock(const std::string_view& vstr)
{
    std::string str{vstr};
    int number = std::stoi(str);
    return static_cast<FileLockType>(number);
}

static inline int8_t parse_lock_depth(const std::string_view& vstr)
{
    std::string str{vstr};
    size_t number = std::stoi(str);
    return static_cast<int8_t>(number);
}

static inline std::chrono::seconds parse_expire_time(const std::string_view& vstr)
{
    std::string str{vstr};
    size_t number = std::stoull(str);
    return std::chrono::seconds{number};
}

static inline auto parse_lock_info(const std::vector<std::string>& list) -> MemoryFileLockService::FileLockMapValueT
{
    return MemoryFileLockService::FileLockMapValueT{list[0], parse_file_lock(list[1]), parse_lock_depth(list[2]),
                                                    parse_expire_time(list[3])};
}

MemoryFileLockService::MemoryFileLockService()
{
    const auto& conf = ConfigReader::GetInstance();
    const auto& data_path = conf.GetLockData();
    const std::string data_path_str = utils::path::to_string(data_path);
    const bool reset = conf.GetWebDavResetLock();

    if (reset)
    {
        if (std::filesystem::exists(data_path))
        {
            if (!std::filesystem::remove(data_path))
            {
                LOG_ERROR("Unable to clear file lock cache.");
            }
        }
    }

    if (std::filesystem::exists(data_path) && !reset)
    {
        std::ifstream ifs{data_path, std::ios::in};
        if (!ifs.is_open())
        {
            LOG_ERROR_FMT("Unable to open '{}', unable to recover file lock cache.", data_path_str)
            return;
        }

        // line string like this: token@path,lock_type,depth,expire_time
        std::string raw_line{};
        while (std::getline(ifs, raw_line))
        {
            const std::string_view line = raw_line;

            auto pos = line.find_last_of('@');
            if (pos == std::string_view::npos)
            {
                continue;
            }

            const auto token = line.substr(0, pos);
            const auto info_str = line.substr(pos + 1);
            if (token.empty() || info_str.empty())
            {
                continue;
            }

            auto info_list = utils::string::split(info_str, ',');
            if (info_list.size() != 4)
            {
                continue;
            }

            try
            {
                lock_map_.insert({FileLockMapKeyT{token}, parse_lock_info(info_list)});
            }
            catch (const std::exception& err)
            {
                LOG_ERROR("The file lock cache may be damaged.");
                LOG_ERROR(err.what());
            }
        }
    }

    data_.open(data_path, std::ios::out);
    if (!data_.is_open())
    {
        LOG_ERROR_FMT("Unable to save file lock cache to '{}', file lock cache are lost when the server stops.",
                      data_path_str);
    }
}

MemoryFileLockService::~MemoryFileLockService()
{
    if (!data_.is_open())
        return;

    // line string like this: token@path,lock_type,depth,expire_time
    for (const auto& [token, data] : lock_map_)
    {
        data_ << utils::path::to_string(token) << '@';
        data_ << data.path << ',';
        data_ << static_cast<int>(data.type) << ',';
        data_ << data.depth << ',';
        data_ << data.expire.count() << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    LOG_INFO_FMT("The file lock cache have been saved to {}", utils::path::to_string(conf.GetLockData()));
}

bool MemoryFileLockService::Lock(const std::string& token, const std::filesystem::path& path, int8_t depth,
                                 FileLockType type, std::chrono::seconds expire_ts)
{
    if (lock_map_.contains(token))
    {
        lock_map_.erase(token);
    }

    lock_map_.insert({FileLockMapKeyT{token}, {path, type, depth, expire_ts}});
    token_map_.insert({path, token});
    return true;
}

bool MemoryFileLockService::Unlock(const std::string& token)
{
    const auto& it = lock_map_.find(token);
    if (it == lock_map_.end())
    {
        return false;
    }

    token_map_.erase(it->second.path);
    lock_map_.erase(it);
    return true;
}

bool MemoryFileLockService::IsLocked(const std::string& token)
{
    using namespace std::chrono;

    const auto& it = lock_map_.find(token);
    if (it == lock_map_.end())
    {
        return false;
    }

    const seconds expire_time = it->second.expire;
    const auto now_tp = system_clock::now();
    const auto now_sec = duration_cast<seconds>(now_tp.time_since_epoch());
    return expire_time > now_sec;
}

bool MemoryFileLockService::IsLocked(const std::filesystem::path& path)
{
    using namespace std::chrono;

    const auto& _it = token_map_.find(path);
    if (_it == token_map_.end())
    {
        return false;
    }

    const auto& it = lock_map_.find(_it->second);
    if (it == lock_map_.end())
    {
        return false;
    }

    const seconds expire_time = it->second.expire;
    const auto now_tp = system_clock::now();
    const auto now_sec = duration_cast<seconds>(now_tp.time_since_epoch());
    return expire_time > now_sec;
}

} // namespace FileLockService
