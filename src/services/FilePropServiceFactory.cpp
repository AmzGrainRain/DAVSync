#include "FilePropServiceFactory.h"

#include "ConfigReader.h"
#include "file_prop/MemoryFilePropService.h"
#include "file_prop/RedisFilePropService.h"
#include "file_prop/SQLiteFilePropService.h"
#include <stdexcept>


namespace FilePropService
{

FilePropService& GetService()
{
    const auto& conf = ConfigReader::GetInstance();

    if (conf.GetPropEngine() == "memory")
    {
        static MemoryFilePropService instance{};
        return instance;
    }

    if (conf.GetPropEngine() == "redis")
    {
        static RedisFilePropService instance{};
        return instance;
    }

    if (conf.GetPropEngine() == "sqlite")
    {
        static SQLiteFilePropService instance{};
        return instance;
    }

    throw std::runtime_error("There is no matching cache engine.");
}

} // namespace FilePropService
