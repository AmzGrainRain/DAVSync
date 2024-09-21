#pragma once
#include "FileETagService.h"

#include <filesystem>
#include <string>

#include <hiredis/hiredis.h>

namespace FileETagService
{

class RedisFileETagService : public FileETagService
{
  public:
    using ReplyT = std::unique_ptr<redisReply, void (*)(void*)>;

    RedisFileETagService();

    std::string Get(const std::string& path_sha) override;

    bool Set(const std::filesystem::path& file) override;

  private:
    ReplyT Exec(const std::string& command);

    std::string auth_str_;
    std::unique_ptr<redisContext> redis_ctx_;
};

} // namespace FileETagService
