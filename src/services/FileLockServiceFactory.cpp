#include "FileLockServiceFactory.h"

#include "file_lock/MemoryFileLockService.h"
#include "file_lock/RedisFileLockService.h"
#include "file_lock/SQLiteFileLockService.h"

#include "ConfigReader.h"

namespace FileLockService
{

FileLockService& GetService()
{
    const auto& conf = ConfigReader::GetInstance();

    if (conf.GetLockEngine() == "memory")
    {
        static MemoryFileLockService instance{};
        return instance;
    }

    if (conf.GetLockEngine() == "redis")
    {
        static RedisFileLockService instance{};
        return instance;
    }

    if (conf.GetLockEngine() == "sqlite")
    {
        static SQLiteFileLockService instance{};
        return instance;
    }

    throw std::runtime_error("There is no matching lock engine.");
}

} // namespace FileLockService
