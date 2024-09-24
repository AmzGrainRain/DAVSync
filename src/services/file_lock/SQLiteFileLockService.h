#include "FileLockService.h"
#include <utility.hpp>

namespace FileLockService
{

REGISTER_AUTO_KEY(FileLockTable, path)

class SQLiteFileLockService : public FileLockService
{
  public:
    SQLiteFileLockService();

    bool Lock(const std::filesystem::path& path, FileLockType type = FileLockType::SHARED,
              std::chrono::seconds expire_time = std::chrono::seconds{0}) override;

    bool Unlock(const std::filesystem::path& path) override;

    bool IsLocked(const std::filesystem::path& path) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FileLockService
