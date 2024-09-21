#pragma once

#include <chrono>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

namespace FileLockeService
{

enum class FileLockType
{
    SHARED = 0,
    EXCLUSIVE
};

struct FileLockTable
{
    std::string etag;
    FileLockType type;
    std::chrono::milliseconds expire_time;
};

class FileLockeService
{
  public:
    virtual bool Lock(const std::string& etag, FileLockType type = FileLockType::SHARED,
                      std::chrono::milliseconds expire_time = std::chrono::milliseconds{0}) = 0;

    virtual bool Unlock(const std::string& etag) = 0;

    virtual bool IsLocked(const std::string& etag) = 0;
};

} // namespace FileLockeService
