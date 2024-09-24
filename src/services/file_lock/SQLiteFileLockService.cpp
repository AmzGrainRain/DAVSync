#include "SQLiteFileLockService.h"

#include <chrono>
#include <filesystem>
#include <format>

#include <spdlog/spdlog.h>
#include <utility>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"

namespace FileLockService
{

REFLECTION(FileLockTable, token, path, type, depth, expire_ts)

SQLiteFileLockService::SQLiteFileLockService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(utils::path::to_string(conf.GetSQLiteDB()).data(), "FileLock"))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileLockTable>(ormpp_key{"token"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool SQLiteFileLockService::Lock(const std::string& token, const std::filesystem::path& path, int8_t depth,
                                 FileLockType type, std::chrono::seconds expire_ts)
{
    FileLockTable col{token, utils::path::to_string(path), static_cast<int>(type), static_cast<int>(depth),
                      expire_ts.count()};
    return dbng_.insert<FileLockTable>(std::move(col)) == 1;
}

bool SQLiteFileLockService::Unlock(const std::string& token)
{
    const std::string where = std::format("token='{}'", token);

    return dbng_.delete_records_s<FileLockTable>(where);
}

bool SQLiteFileLockService::IsLocked(const std::string& token)
{
    const std::string where = std::format("token='{}'", token);
    const auto query_res = dbng_.query_s<FileLockTable>(where);
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.");
        return false;
    }

    auto now_sec = utils::get_timestamp<std::chrono::seconds>();
    std::chrono::seconds expire_time{query_res[0].expire_ts};
    return expire_time > now_sec;
}

bool SQLiteFileLockService::IsLocked(const std::filesystem::path& path)
{
    const std::string path_str = utils::path::to_string(path);
    const std::string where = std::format("path='{}'", path_str);
    const auto query_res = dbng_.query_s<FileLockTable>(where);
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.");
        return false;
    }

    auto now_sec = utils::get_timestamp<std::chrono::seconds>();
    std::chrono::seconds expire_time{query_res[0].expire_ts};
    return expire_time > now_sec;
}

} // namespace FileLockService
