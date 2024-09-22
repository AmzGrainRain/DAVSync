#pragma once

#include <chrono>
#include <filesystem>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

namespace FileLockService
{

enum class FileLockType
{
    SHARED = 0,
    EXCLUSIVE
};

struct FileLockTable
{
    std::string path_sha;
    FileLockType type;
    std::chrono::seconds expire_time;
};

class FileLockService
{
  public:
    virtual bool Lock(const std::filesystem::path& path, FileLockType type = FileLockType::SHARED,
                      std::chrono::seconds expire_time = std::chrono::seconds{0}) = 0;

    virtual bool Unlock(const std::filesystem::path& path) = 0;

    virtual bool IsLocked(const std::filesystem::path& path) = 0;
};

} // namespace FileLockService
