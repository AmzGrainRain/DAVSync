#include "MemoryFileLockService.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"
#include "utils/string.h"

static inline FileLockService::FileLockType ParseFileLock(const std::string_view& vstr)
{
    std::string str{vstr};
    int number = std::stoi(str);
    return static_cast<FileLockService::FileLockType>(number);
}

static inline int8_t ParseLockDepth(const std::string_view& vstr)
{
    std::string str{vstr};
    size_t number = std::stoi(str);
    return static_cast<int8_t>(number);
}

static inline std::chrono::seconds ParseLockExpiresTime(const std::string_view& vstr)
{
    std::string str{vstr};
    size_t number = std::stoull(str);
    return std::chrono::seconds{number};
}

static inline auto ParseLockInfo(const std::vector<std::string>& list) -> FileLockService::FileLock
{
    using namespace FileLockService;

    std::string token = list[0];
    std::string path = list[1];
    short depth = static_cast<short>(std::stoi(list[2]));
    int scope = std::stoi(list[3]);
    int type = std::stoi(list[4]);
    long long expires_at = std::stoll(list[5]);
    long long creation_date = std::stoll(list[6]);

    return FileLock{std::move(token), std::move(path), depth, scope, type, expires_at, creation_date};
}

namespace FileLockService
{

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

        // line: token@path, depth, scope, type, expires_at, creation_date
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
                lock_map_.insert({FileLockMapKeyT{token}, ParseLockInfo(info_list)});
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
        LOG_ERROR_FMT("Unable to save file lock cache to '{}', file lock cache are lost when the server stops.", data_path_str);
    }
}

MemoryFileLockService::~MemoryFileLockService()
{
    if (!data_.is_open())
        return;

    // line: token@path, depth, scope, type, expires_at, creation_date
    for (const auto& [token, data] : lock_map_)
    {
        data_ << data.token << '@';
        data_ << data.path << ',';
        data_ << data.depth << ',';
        data_ << data.scope << ',';
        data_ << data.type << ',';
        data_ << data.expires_at << ',';
        data_ << data.creation_date << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    LOG_INFO_FMT("The file lock cache have been saved to {}", utils::path::to_string(conf.GetLockData()));
}

bool MemoryFileLockService::Lock(const FileLock& lock) noexcept
{
    if (lock_map_.contains(lock.token))
    {
        lock_map_.erase(lock.token);
    }

    token_map_.insert({lock.path, lock.token});
    lock_map_.insert({lock.token, lock});
    return true;
}

bool MemoryFileLockService::Lock(FileLock&& lock) noexcept
{
    if (lock_map_.contains(lock.token))
    {
        lock_map_.erase(lock.token);
    }

    token_map_.insert({lock.path, lock.token});
    lock_map_.insert({lock.token, std::forward<FileLock&&>(lock)});
    return true;
}

bool MemoryFileLockService::Unlock(const std::string& token) noexcept
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

bool MemoryFileLockService::IsLocked(const std::string& token) noexcept
{
    using namespace std::chrono;

    const auto& it = lock_map_.find(token);
    if (it == lock_map_.end())
    {
        return false;
    }

    const seconds expires_sec = it->second.ExpiresAt();
    const seconds now_sec = utils::get_timestamp<std::chrono::seconds>();
    return expires_sec > now_sec;
}

bool MemoryFileLockService::IsLocked(const std::filesystem::path& path) noexcept
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

    const seconds expires_sec = it->second.ExpiresAt();
    const seconds now_sec = utils::get_timestamp<std::chrono::seconds>();
    return expires_sec > now_sec;
}

FileLock MemoryFileLockService::GetLock(const std::string& token) noexcept(false)
{
    const auto& it = lock_map_.find(token);
    if (it == lock_map_.end())
    {
        throw std::runtime_error("lock not exists.");
    }

    return it->second;
}

FileLock MemoryFileLockService::GetLock(const std::filesystem::path& path) noexcept(false)
{
    return GetLock(utils::path::to_string(path));
}

} // namespace FileLockService
