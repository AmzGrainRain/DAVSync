#include "FileLockService.h"

#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <utility>

namespace FileLockService
{

class MemoryFileLockService : public FileLockService
{
  public:
    using FileLockMapKeyT = std::filesystem::path;
    using FileLockMapValueT = std::pair<FileLockType, std::chrono::seconds>;
    using FileLockMapT = std::unordered_map<FileLockMapKeyT, FileLockMapValueT>;

    MemoryFileLockService();

    ~MemoryFileLockService();

    bool Lock(const std::filesystem::path& path, FileLockType type = FileLockType::SHARED,
              std::chrono::seconds expire_time = std::chrono::seconds{0}) override;

    bool Unlock(const std::filesystem::path& path) override;

    bool IsLocked(const std::filesystem::path& path) override;

  private:
    FileLockMapT lock_map_;
    std::ofstream data_;
};

} // namespace FileLockService
