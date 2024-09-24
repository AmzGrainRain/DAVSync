#include "FileLockService.h"
#include <utility.hpp>

namespace FileLockService
{

REGISTER_AUTO_KEY(FileLockTable, path)

class SQLiteFileLockService : public FileLockService
{
  public:
    SQLiteFileLockService();

    bool Lock(const std::string& token, const std::filesystem::path& path, int8_t depth, FileLockType type,
              std::chrono::seconds expire_ts) override;

    bool Unlock(const std::string& token) override;

    bool IsLocked(const std::string& token) override;

    bool IsLocked(const std::filesystem::path& path) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FileLockService
