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

bool FileLock::IsShared() const
{
    return static_cast<FileLockScope>(scope) == FileLockScope::SHARED;
}

bool FileLock::IsExclusive() const
{
    return static_cast<FileLockScope>(scope) == FileLockScope::EXCLUSIVE;
}

bool FileLock::IsWriteLock() const
{
    return static_cast<FileLockType>(type) == FileLockType::WRITE;
}

bool FileLock::IsReadLock() const
{
    return static_cast<FileLockType>(type) == FileLockType::READ;
}

} // namespace FileLockService
