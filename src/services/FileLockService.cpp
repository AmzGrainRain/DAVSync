#include "FileLockService.h"

#include <format>
#include <stdexcept>

#include <iguana/reflection.hpp>
#include <ormpp/entity.hpp>
#include <string>

#include "ConfigReader.h"

REFLECTION(FileLockTableRow, etag, type, expire_time);

FileLockerService& FileLockerService::GetInstance()
{
    static FileLockerService instance{};
    return instance;
}

FileLockerService::FileLockerService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(conf.GetSQLiteLocation()))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    // etag is PRIMARY_KEY
    if (!dbng_.create_datatable<FileLockTableRow>(ormpp_key{"etag"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool FileLockerService::Lock(const std::string& etag, FileLockType type, std::chrono::milliseconds expire_time)
{
    return dbng_.insert<FileLockTableRow>({etag, type, expire_time}) == 1;
}

bool FileLockerService::Unlock(const std::string& etag)
{
    if (!IsLocked(etag))
        return false;

    return dbng_.delete_records_s<FileLockTableRow>(std::format("etag={}", etag));
}

bool FileLockerService::ModifyLock(const std::string& etag, FileLockType type)
{
    if (!IsLocked(etag))
        return false;

    auto res = dbng_.query_s<FileLockTableRow>(std::format("etag={}", etag));
    res[0].type = type;
    return dbng_.update<FileLockTableRow>(res[0]);
}

bool FileLockerService::IsLocked(const std::string& etag)
{
    auto res = dbng_.query_s<FileLockTableRow>(std::format("etag={}", etag));
    return res.size() == 1;
}
