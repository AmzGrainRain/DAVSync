#include "FileLockService.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace FileLockService
{

class MemoryFileLockService : public FileLockService
{
  public:
    using FileLockMapKeyT = std::string;
    using FileLockMapT = std::unordered_map<FileLockMapKeyT, FileLock>;

    MemoryFileLockService();

    ~MemoryFileLockService();

    bool Lock(const FileLock& lock) noexcept override;

    bool Lock(FileLock&& lock) noexcept override;

    bool Unlock(const std::string& token) noexcept override;

    bool IsLocked(const std::string& token) noexcept override;

    bool IsLocked(const std::filesystem::path& path) noexcept override;

    FileLock GetLock(const std::string& token) noexcept(false) override;

    FileLock GetLock(const std::filesystem::path& path) noexcept(false) override;

  private:
    FileLockMapT lock_map_;
    std::unordered_map<std::filesystem::path, std::string> token_map_;
    std::ofstream data_;
};

} // namespace FileLockService
