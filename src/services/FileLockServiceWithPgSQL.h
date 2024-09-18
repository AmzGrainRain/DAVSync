#pragma once

#include "FileLockService.h"

class FileLockServiceWithPgSQL : public FileLockService
{
  public:
    bool LockFile(const std::string& lock_token, FileLockType lock_type,
                  std::chrono::seconds lock_expire_time) const override
    {
        return false;
    }

    bool UnlockFile(const std::string& lock_token) const override
    {
        return false;
    }

    bool IsLocked(const std::string& lock_token) const override
    {
        return false;
    }
};
