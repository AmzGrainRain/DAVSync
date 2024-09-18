#pragma once

#include <chrono>
#include <string>

enum class FileLockType
{
    NONE = 0,
    SHARED,
    EXCLUSIVE
};

class FileLockService
{
  public:
    virtual ~FileLockService() {};
    virtual bool LockFile(const std::string& lock_token, FileLockType lock_type,
                          std::chrono::seconds lock_expire_time) const = 0;
    virtual bool UnlockFile(const std::string& lock_token) const = 0;
    virtual bool IsLocked(const std::string& lock_token) const = 0;
};
