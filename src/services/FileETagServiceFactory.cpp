#include "FileETagServiceFactory.h"

#include <stdexcept>

#include "file_etag/MemoryFileETagService.h"
#include "file_etag/RedisFileETagService.h"
#include "file_etag/SQLiteFileETagService.h"
#include "ConfigReader.h"

namespace FileETagService
{

FileETagService& GetService()
{
    const auto& conf = ConfigReader::GetInstance();
    auto engine = conf.GetETagEngine();

    if (engine == "memory")
    {
        static MemoryFileETagService instance{};
        return instance;
    }

    if (engine == "redis")
    {
        static RedisFileETagService instance{};
        return instance;
    }

    if (engine == "sqlite")
    {
        static SQLiteFileETagService instance{};
        return instance;
    }

    throw std::runtime_error("There is no matching cache engine.");
}

} // namespace FileETagService
