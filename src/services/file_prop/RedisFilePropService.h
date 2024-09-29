#pragma once
#include "FilePropService.h"

#include <filesystem>

#include <hiredis/hiredis.h>

#include "utils/redis.h"

namespace FilePropService
{

class RedisFilePropService : public FilePropService
{
  public:
    RedisFilePropService() noexcept(false);

    bool Set(const std::filesystem::path& path, const PropT& prop) noexcept override;

    std::string Get(const std::filesystem::path& path, const std::string& key) noexcept override;

    std::vector<PropT> GetAll(const std::filesystem::path& path) noexcept override;

    bool Remove(const std::filesystem::path& path, const std::string& key) noexcept override;

    bool RemoveAll(const std::filesystem::path& path) noexcept override;

  private:
    std::string auth_str_;
    utils::redis::RedisContextT redis_ctx_;
};

} // namespace FilePropService
