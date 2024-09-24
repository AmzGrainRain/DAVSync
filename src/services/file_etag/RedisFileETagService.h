#pragma once
#include "FileETagService.h"

#include <filesystem>
#include <string>

#include <hiredis/hiredis.h>

#include "utils/redis.h"

namespace FileETagService
{

class RedisFileETagService : public FileETagService
{
  public:
    RedisFileETagService();

    std::string Get(const std::filesystem::path& path) override;

    std::string Set(const std::filesystem::path& path) override;

  private:
    std::string auth_str_;
    utils::redis::RedisContextT redis_ctx_;
};

} // namespace FileETagService
