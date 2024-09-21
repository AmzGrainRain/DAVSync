#pragma once
#include "FilePropService.h"

#include <memory>

#include <hiredis/hiredis.h>

namespace FilePropService
{

class RedisFilePropService : public FilePropService
{
  public:
    using ReplyT = std::unique_ptr<redisReply, void (*)(void*)>;

    RedisFilePropService();

    bool Set(const std::string& path_sha, const PropT& prop) override;

    std::string Get(const std::string& path_sha, const std::string& key) override;

    std::vector<PropT> GetAll(const std::string& path_sha) override;

    bool Remove(const std::string& path_sha, const std::string& key) override;

    bool RemoveAll(const std::string& path_sha) override;

  private:
    ReplyT Exec(const std::string& command);

    std::string auth_str_;
    std::unique_ptr<redisContext> redis_ctx_;
};

} // namespace FilePropService
