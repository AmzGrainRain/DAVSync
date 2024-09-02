#include "FilePropServiceFactory.h"
#include <stdexcept>

#include "ConfigReader.h"
#include "FilePropService_Redis.h"
#include "FilePropService_SQLite.h"

FilePropService& FilePropServiceFactory()
{
    const auto& conf = ConfigReader::GetInstance();

    if (conf.GetSQLiteEnable())
    {
        static FilePropService_SQLite instance{};
        return instance;
    }

    if (conf.GetRedisEnable())
    {
        static FilePropService_Redis instance{};
        return instance;
    }

    throw std::runtime_error("How did you successfully start an instance with an incorrect configuration?");
}
