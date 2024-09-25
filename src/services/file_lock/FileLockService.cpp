#include "FileLockService.h"

namespace FileLockService
{

std::filesystem::path FileLock::GetPath() const
{
    return std::filesystem::path{path};
}

FileLockScope FileLock::GetScope() const
{
    return static_cast<FileLockScope>(scope);
}

FileLockType FileLock::GetType() const
{
    return static_cast<FileLockType>(type);
}

std::chrono::seconds FileLock::ExpiresAt() const
{
    return std::chrono::seconds{expires_at};
}

std::chrono::seconds FileLock::CreationDate() const
{
    return std::chrono::seconds{creation_date};
}

} // namespace FileLockService
