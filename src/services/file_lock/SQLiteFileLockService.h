#include "FileLockService.h"
#include <utility.hpp>

namespace FileLockService
{

class SQLiteFileLockService : public FileLockService
{
  public:
    SQLiteFileLockService();

    bool Lock(const FileLock& lock) override;

    bool Lock(FileLock&& lock) override;

    bool Unlock(const std::string& token) override;

    bool IsLocked(const std::string& token) override;

    bool IsLocked(const std::filesystem::path& path) override;

    FileLock GetLock(const std::string& token) override;

    FileLock GetLock(const std::filesystem::path& path) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FileLockService
