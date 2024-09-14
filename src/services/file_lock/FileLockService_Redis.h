#pragma once

#include "FileLockService.h"

class FileLockService_Redis : public FileLockService
{
  public:
    FileLockService_Redis();

    FileLockService_Redis(const sw::redis::ConnectionOptions& client_options);

    bool LockFile(const std::filesystem::path& file_path);

    bool UnlockFile(const std::filesystem::path& file_path);

    bool IsLocked(const std::filesystem::path& file_path);
};
