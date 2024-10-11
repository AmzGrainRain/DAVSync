#include <filesystem>
#include <list>
#include <limits>

#include "utils.h"

namespace FileLock
{

enum class LockScope
{
    SHARED = 0,
    EXCLUSIVE
};

enum class LockType
{
    WRITE = 0,
    READ
};

struct EntryLock
{
    std::string token;
    short depth = std::numeric_limits<short>::min();
    LockScope scope = LockScope::SHARED;
    LockType type = LockType::WRITE;
    long long expires_at = 0;
    long long creation_date = utils::get_timestamp<std::chrono::seconds>().count();

    EntryLock() = default;

    EntryLock(const EntryLock& lock);

    EntryLock(EntryLock&& lock);

    ~EntryLock() = default;

    std::chrono::seconds ExpiresAt() const;

    std::chrono::seconds CreationDate() const;
};

class Service
{
  public:
    static Service& GetInstance();

    void Lock(const std::filesystem::path& path, const EntryLock& lock);

    bool Unlock(const std::filesystem::path& path);

    const EntryLock* GetLock(const std::filesystem::path& path);

    bool IsLocked(const std::filesystem::path& path, bool by_parent = true);

    bool LockExists(const std::filesystem::path& path);

  private:
    struct Entry
    {
        std::string name = "";
        std::filesystem::path path = "/";

        Entry* parent = nullptr;
        EntryLock* lock = nullptr;
        std::list<Entry*>* children = nullptr;

        Entry() = default;
        ~Entry();
    };

    Service();

    ~Service();

    Entry* FindEntry(const std::filesystem::path& path);

    Entry* FindLock(const std::filesystem::path& path, bool by_parent = true);

    Entry* root_ = nullptr;
};

} // namespace FileLock
