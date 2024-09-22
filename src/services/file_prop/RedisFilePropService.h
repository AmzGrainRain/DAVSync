#pragma once
#include "FilePropService.h"

#include <filesystem>
#include <memory>

#include <hiredis/hiredis.h>

namespace FilePropService
{

class RedisFilePropService : public FilePropService
{
  public:
    using ReplyT = std::unique_ptr<redisReply, void (*)(void*)>;

    RedisFilePropService();

    bool Set(const std::filesystem::path& path, const PropT& prop) override;

    std::string Get(const std::filesystem::path& path, const std::string& key) override;

    std::vector<PropT> GetAll(const std::filesystem::path& path) override;

    bool Remove(const std::filesystem::path& path, const std::string& key) override;

    bool RemoveAll(const std::filesystem::path& path) override;

  private:
    ReplyT Exec(const std::string& command);

    std::string auth_str_;
    std::unique_ptr<redisContext> redis_ctx_;
};

} // namespace FilePropService
