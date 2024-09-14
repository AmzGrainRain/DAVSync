#include "FileLockService_Redis.h"

#include <sw/redis++/redis++.h>

#include "ConfigReader.h"

FileLockService_Redis::FileLockService_Redis()
{
    const auto& conf = ConfigReader::GetInstance();
    // TODO
}

FileLockService_Redis::FileLockService_Redis(const sw::redis::ConnectionOptions& client_options)
:client_(client_options) {}

bool FileLockService_Redis::LockFile(const std::filesystem::path& file_path)
{
    // TODO
}

bool FileLockService_Redis::UnlockFile(const std::filesystem::path& file_path)
{
    // TODO
}

bool FileLockService_Redis::IsLocked(const std::filesystem::path& file_path)
{
    // TODO
}
