#pragma once

#include <chrono>
#include <filesystem>
#include <string>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

#include "utils.h"

namespace FileLockService
{

enum class FileLockScope
{
    SHARED = 0,
    EXCLUSIVE
};

enum class FileLockType
{
    WRITE = 0,
    READ
};

// token, path, depth, scope, type, expires_at, creation_date
struct FileLock
{
    std::string token;
    std::string path;
    short depth = 1;
    int scope = static_cast<int>(FileLockScope::SHARED);
    int type = static_cast<int>(FileLockType::WRITE);
    long long expires_at = 0;
    long long creation_date = utils::get_timestamp<std::chrono::seconds>().count();

    std::filesystem::path GetPath() const;

    FileLockScope GetScope() const;

    FileLockType GetType() const;

    std::chrono::seconds ExpiresAt() const;

    std::chrono::seconds CreationDate() const;
};

class FileLockService
{
  public:
    virtual bool Lock(const FileLock& lock) = 0;

    virtual bool Lock(FileLock&& lock) = 0;

    virtual bool Unlock(const std::string& token) = 0;

    virtual bool IsLocked(const std::string& token) = 0;
    virtual bool IsLocked(const std::filesystem::path& path) = 0;

    virtual FileLock GetLock(const std::string& token) = 0;
    virtual FileLock GetLock(const std::filesystem::path& path) = 0;
};

} // namespace FileLockService
