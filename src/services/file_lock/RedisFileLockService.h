#include "FileLockService.h"

#include <filesystem>
#include <string>
#include <unordered_map>

#include <hiredis/hiredis.h>

#include "utils/redis.h"

namespace FileLockService
{

class RedisFileLockService : public FileLockService
{
  public:
    RedisFileLockService();

    bool Lock(const FileLock& lock) noexcept override;

    bool Lock(FileLock&& lock) noexcept override;

    bool Unlock(const std::string& token) noexcept override;

    bool IsLocked(const std::string& token) noexcept override;

    bool IsLocked(const std::filesystem::path& path) noexcept override;

    FileLock GetLock(const std::string& token) noexcept(false) override;

    FileLock GetLock(const std::filesystem::path& path) noexcept(false) override;

  private:
    std::string auth_str_;
    utils::redis::RedisContextT redis_ctx_;
    std::unordered_map<std::filesystem::path, std::string> token_map_;
};

} // namespace FileLockService
