#pragma once

#include "FileLockService.h"

class FileLockService_SQLite : public FileLockService
{
  public:
    bool LockFile(const std::filesystem::path& file_path)
    {
        // TODO
        return false;
    }

    bool UnlockFile(const std::filesystem::path& file_path)
    {
        // TODO
        return false;
    }

    bool IsLocked(const std::filesystem::path& file_path)
    {
        // TODO
        return false;
    }
};
