#pragma once

#include <chrono>

#include <mutex>
#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

enum class FileLockType
{
    SHARED = 0,
    EXCLUSIVE
};

struct FileLockTableRow
{
    std::string etag;
    FileLockType type;
    std::chrono::milliseconds expire_time;
};

class FileLockerService
{
  public:
    static FileLockerService& GetInstance();

    bool Lock(const std::string& etag, FileLockType type = FileLockType::SHARED,
              std::chrono::milliseconds expire_time = std::chrono::milliseconds{0});

    bool Unlock(const std::string& etag);

    bool ModifyLock(const std::string& etag, FileLockType type);

    bool IsLocked(const std::string& etag);

  private:
    FileLockerService();

    ormpp::dbng<ormpp::sqlite> dbng_;
};
