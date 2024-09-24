#include "SQLiteFileLockService.h"

#include <chrono>
#include <cstddef>
#include <format>

#include <spdlog/spdlog.h>

#include "logger.hpp"
#include "ConfigReader.h"
#include "utils.h"
#include "utils/path.h"

namespace FileLockService
{

REFLECTION(FileLockTable, path, type, expire_time)

SQLiteFileLockService::SQLiteFileLockService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(conf.GetSQLiteDB()))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileLockTable>(ormpp_key{"path"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool SQLiteFileLockService::Lock(const std::filesystem::path& path, FileLockType type, std::chrono::seconds expire_time)
{
    const std::string path_str = utils::path::to_string(path);
    const int lock_type = static_cast<int>(type);
    const size_t expire_time_sec = expire_time.count();

    return dbng_.insert<FileLockTable>({path_str, lock_type, expire_time_sec}) == 1;
}

bool SQLiteFileLockService::Unlock(const std::filesystem::path& path)
{
    const std::string path_str = utils::path::to_string(path);
    const std::string where = std::format("path='{}'", path_str);

    return dbng_.delete_records_s<FileLockTable>(where);
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
    std::chrono::seconds expire_time{query_res[0].expire_time};
    return expire_time > now_sec;
}

} // namespace FileLockService
