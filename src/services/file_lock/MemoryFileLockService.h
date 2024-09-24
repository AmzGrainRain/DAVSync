#include "FileLockService.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace FileLockService
{

class MemoryFileLockService : public FileLockService
{
  public:
    using FileLockMapKeyT = std::string;
    using FileLockMapValueT = struct
    {
        std::filesystem::path path;
        FileLockType type;
        int8_t depth;
        std::chrono::seconds expire;
    };
    using FileLockMapT = std::unordered_map<FileLockMapKeyT, FileLockMapValueT>;

    MemoryFileLockService();

    ~MemoryFileLockService();

    bool Lock(const std::string& token, const std::filesystem::path& path, int8_t depth, FileLockType type,
              std::chrono::seconds expire_ts) override;

    bool Unlock(const std::string& token) override;

    bool IsLocked(const std::string& token) override;

    bool IsLocked(const std::filesystem::path& path) override;

  private:
    FileLockMapT lock_map_;
    std::unordered_map<std::filesystem::path, std::string> token_map_;
    std::ofstream data_;
};

} // namespace FileLockService
