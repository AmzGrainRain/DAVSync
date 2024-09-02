#pragma once

#include <filesystem>

class FileLockService
{
  public:
    virtual bool LockFile(const std::filesystem::path& file_path) = 0;
    virtual bool UnlockFile(const std::filesystem::path& file_path) = 0;
    virtual bool IsLocked(const std::filesystem::path& file_path) = 0;
};
