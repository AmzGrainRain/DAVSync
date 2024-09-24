#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>
#include <string>

namespace FileLockService
{

enum class FileLockType
{
    SHARED = 0,
    EXCLUSIVE
};

struct FileLockTable
{
    std::string token;
    std::string path;
    int type;
    int depth;
    long long expire_ts;
}; // token, path, type, depth, expire_ts

class FileLockService
{
  public:
    virtual bool Lock(const std::string& token, const std::filesystem::path& path, int8_t depth = 1,
                      FileLockType type = FileLockType::SHARED,
                      std::chrono::seconds expire_ts = std::chrono::seconds{0}) = 0;

    virtual bool Unlock(const std::string& token) = 0;

    virtual bool IsLocked(const std::string& token) = 0;

    virtual bool IsLocked(const std::filesystem::path& path) = 0;
};

} // namespace FileLockService
