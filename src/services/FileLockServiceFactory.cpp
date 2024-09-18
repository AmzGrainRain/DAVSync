#include "FileLockServiceFactory.h"

#include "ConfigReader.h"
#include "FileLockServiceWithPgSQL.h"
#include "FileLockServiceWithRedis.h"
#include "FileLockServiceWithSQLite.h"

FileLockService& FileLockServiceFactory()
{
    const auto& conf = ConfigReader::GetInstance();

    if (conf.GetSQLiteEnable())
    {
        static FileLockServiceWithSQLite instance{};
        return instance;
    }
    else if (conf.GetRedisEnable())
    {
        static FileLockServiceWithPgSQL instance{};
        return instance;
    }
    else if (conf.GetRedisEnable())
    {
        static FileLockServiceWithRedis instance{};
        return instance;
    }

    throw std::runtime_error("How did you successfully start an instance with an incorrect configuration?");
}
