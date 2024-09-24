#include "FileLockService.h"

#include <chrono>
#include <filesystem>
#include <string>

#include <hiredis/hiredis.h>

#include "utils/redis.h"

namespace FileLockService
{

class RedisFileLockService : public FileLockService
{
  public:
    RedisFileLockService();

    bool Lock(const std::filesystem::path& path, FileLockType type = FileLockType::SHARED,
              std::chrono::seconds expire_time = std::chrono::seconds{0}) override;

    bool Unlock(const std::filesystem::path& path) override;

    bool IsLocked(const std::filesystem::path& path) override;

  private:
    std::string auth_str_;
    utils::redis::RedisContextT redis_ctx_;
};

} // namespace FileLockService
