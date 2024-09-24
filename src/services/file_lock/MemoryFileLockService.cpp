#include "MemoryFileLockService.h"

#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include "logger.hpp"
#include "ConfigReader.h"
#include "FileLockService.h"
#include "utils/path.h"

namespace FileLockService
{

static inline FileLockType parse_file_lock(const std::string_view& vstr)
{
    std::string str{vstr};
    int number = std::stoi(str);
    return static_cast<FileLockType>(number);
}

static inline std::chrono::seconds parse_expire_time(const std::string_view& vstr)
{
    std::string str{vstr};
    size_t number = std::stoull(str);
    return std::chrono::seconds{number};
}

MemoryFileLockService::MemoryFileLockService()
{
    const auto& conf = ConfigReader::GetInstance();
    const auto& data_path = conf.GetLockData();
    const std::string data_path_str = utils::path::to_string(data_path);
    const bool reset = conf.GetWebDavResetLock();

    if (reset)
    {
        if(std::filesystem::exists(data_path))
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

        // line string like this: path,lock_type@expire_time
        std::string raw_line{};
        while (std::getline(ifs, raw_line))
        {
            const std::string_view line = raw_line;

            auto pos1 = line.find_last_of(',');
            if (pos1 == std::string_view::npos)
            {
                continue;
            }

            auto pos2 = line.find_last_of('@');
            if (pos2 == std::string_view::npos)
            {
                continue;
            }

            const auto path = line.substr(0, pos1);
            const auto lock_type = line.substr(pos1 + 1, pos2);
            const auto expire_time = line.substr(pos2 + 1);
            if (path.empty() || lock_type.empty() || expire_time.empty())
            {
                continue;
            }

            try
            {
                FileLockMapValueT value{parse_file_lock(lock_type), parse_expire_time(expire_time)};
                lock_map_.insert({FileLockMapKeyT{path}, std::move(value)});
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

    for (const auto& [path, lock] : lock_map_)
    {
        data_ << utils::path::to_string(path) << ',' << static_cast<int>(lock.first) << '@' << lock.second.count()
              << std::endl;
    }

    const auto& conf = ConfigReader::GetInstance();
    LOG_INFO_FMT("The file lock cache have been saved to {}", utils::path::to_string(conf.GetLockData()));
}

bool MemoryFileLockService::Lock(const std::filesystem::path& path, FileLockType type, std::chrono::seconds expire_time)
{
    if (lock_map_.contains(path))
    {
        lock_map_.erase(path);
    }

    FileLockMapValueT value{type, expire_time.count()};
    lock_map_.insert({FileLockMapKeyT{path}, std::move(value)});
    return true;
}

bool MemoryFileLockService::Unlock(const std::filesystem::path& path)
{
    if (!lock_map_.contains(path))
    {
        return false;
    }

    return static_cast<size_t>(lock_map_.erase(path)) == 1;
}

bool MemoryFileLockService::IsLocked(const std::filesystem::path& path)
{
    using namespace std::chrono;

    const auto& it = lock_map_.find(path);
    if (it == lock_map_.end())
    {
        return false;
    }

    const seconds expire_time = it->second.second;
    const auto now_tp = system_clock::now();
    const auto now_sec = duration_cast<seconds>(now_tp.time_since_epoch());
    return expire_time > now_sec;
}

} // namespace FileLockService
