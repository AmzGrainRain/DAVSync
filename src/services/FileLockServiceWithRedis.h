#pragma once

#include "FileLockService.h"

#include <hiredis/async.h>
#include <hiredis/hiredis.h>

using RedisContext = std::unique_ptr<redisContext, decltype(&redisFree)>;
using RedisReply = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

class FileLockServiceWithRedis : public FileLockService
{
  public:
    FileLockServiceWithRedis();

    bool LockFile(const std::string& lock_token, FileLockType lock_type,
                  std::chrono::seconds lock_expire_time) const override;

    bool UnlockFile(const std::string& lock_token) const override;

    bool IsLocked(const std::string& lock_token) const override;

  private:
    redisOptions redis_options_;
};
