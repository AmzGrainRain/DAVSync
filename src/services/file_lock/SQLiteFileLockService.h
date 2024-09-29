#include "FileLockService.h"
#include <utility.hpp>

namespace FileLockService
{

class SQLiteFileLockService : public FileLockService
{
  public:
    SQLiteFileLockService();

    bool Lock(const FileLock& lock) noexcept override;

    bool Lock(FileLock&& lock) noexcept override;

    bool Unlock(const std::string& token) noexcept override;

    bool IsLocked(const std::string& token) noexcept override;

    bool IsLocked(const std::filesystem::path& path) noexcept override;

    FileLock GetLock(const std::string& token) noexcept(false) override;

    FileLock GetLock(const std::filesystem::path& path) noexcept(false) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FileLockService
