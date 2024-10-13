#include <filesystem>
#include <limits>
#include <list>
#include <unordered_map>

#include "utils.h"

namespace FileLock
{

namespace fs = std::filesystem;

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
    std::string user;
    std::string token;
    std::string description;
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
  private:
    struct Entry;

  public:
    using LockListT = std::unordered_map<std::string, EntryLock*>;
    using EntryListT = std::list<Entry*>;

    static Service& GetInstance();

    bool Lock(const fs::path& path, const EntryLock& lock);

    bool Lock(const fs::path& path, EntryLock&& lock);

    bool Unlock(const fs::path& path, const std::string& user);

    const EntryLock* GetLock(const fs::path& path, const std::string& user);

    const LockListT* GetAllLock(const fs::path& path);

    bool IsLocked(const fs::path& path, bool by_parent = true);

    bool HoldingExclusiveLock(const fs::path& path);

  private:
    struct Entry
    {
        std::string name = "";
        fs::path path = "/";

        Entry* parent = nullptr;
        LockListT* lock = nullptr;
        EntryListT* children = nullptr;

        Entry() = default;
        ~Entry();
    };

    Service();

    ~Service();

    Entry* FindEntry(const fs::path& path);

    Entry* FindLock(const fs::path& path, bool by_parent = true);

    Entry* root_ = nullptr;
};

} // namespace FileLock
