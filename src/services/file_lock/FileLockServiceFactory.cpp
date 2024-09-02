#include "FileLockServiceFactory.h"

#include "ConfigReader.h"
#include "FileLockService_Redis.h"
#include "FileLockService_SQLite.h"

FileLockService& FileLockServiceFactory()
{
    const auto& conf = ConfigReader::GetInstance();

    if (conf.GetSQLiteEnable())
    {
        static FileLockService_SQLite instance{};
        return instance;
    }

    if (conf.GetRedisEnable())
    {
        static FileLockService_Redis instance{};
        return instance;
    }

    throw std::runtime_error("How did you successfully start an instance with an incorrect configuration?");
}
