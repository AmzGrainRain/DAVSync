#include "SQLiteFileLockService.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <stdexcept>

#include "ConfigReader.h"
#include "FileLockService.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"

namespace FileLockService
{

REFLECTION(FileLock, token, path, depth, scope, type, expires_at, creation_date)

SQLiteFileLockService::SQLiteFileLockService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(utils::path::to_string(conf.GetSQLiteDB()).data(), "FileLock"))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileLock>(ormpp_key{"token"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool SQLiteFileLockService::Lock(const FileLock& lock) noexcept
{
    return dbng_.insert<FileLock>(lock) == 1;
}

bool SQLiteFileLockService::Lock(FileLock&& lock) noexcept
{
    return Lock(lock);
}

bool SQLiteFileLockService::Unlock(const std::string& token) noexcept
{
    const std::string where = std::format("token='{}'", token);

    return dbng_.delete_records_s<FileLock>(where);
}

bool SQLiteFileLockService::IsLocked(const std::string& token) noexcept
{
    const std::string where = std::format("token='{}'", token);
    const auto query_res = dbng_.query_s<FileLock>(where);
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.");
        return false;
    }

    const auto expires_sec = query_res[0].ExpiresAt();
    const auto now_sec = utils::get_timestamp<std::chrono::seconds>();
    return expires_sec > now_sec;
}

bool SQLiteFileLockService::IsLocked(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string where = std::format("path='{}'", path_str);
    const auto query_res = dbng_.query_s<FileLock>(where);
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.");
        return false;
    }

    const auto expires_sec = query_res[0].ExpiresAt();
    const auto now_sec = utils::get_timestamp<std::chrono::seconds>();
    return expires_sec > now_sec;
}

FileLock SQLiteFileLockService::GetLock(const std::string& token) noexcept(false)
{
    const std::string where = std::format("token='{}'", token);
    const auto query_res = dbng_.query_s<FileLock>(where);
    if (query_res.size() != 1)
    {
        throw std::runtime_error("The database may be damaged.");
    }

    return query_res[0];
}

FileLock SQLiteFileLockService::GetLock(const std::filesystem::path& path) noexcept(false)
{
    const std::string path_str = utils::path::to_string(path);
    const std::string where = std::format("path='{}'", path_str);
    const auto query_res = dbng_.query_s<FileLock>(where);
    if (query_res.size() != 1)
    {
        throw std::runtime_error("The database may be damaged.");
    }

    return query_res[0];
}

} // namespace FileLockService
