#include "FileLockService.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "utils.h"

namespace FileLock
{

/*****************/
/*  struct Lock  */
/*****************/
EntryLock::EntryLock(const EntryLock& lock)
    : user(lock.user), token(lock.token), description(lock.description), depth(lock.depth), scope(lock.scope), type(lock.type),
      expires_at(lock.expires_at), creation_date(lock.creation_date) {};

EntryLock::EntryLock(EntryLock&& lock)
    : user(std::move(lock.user)), token(std::move(lock.token)), description(std::move(lock.description)), depth(lock.depth), scope(lock.scope),
      type(lock.type), expires_at(lock.expires_at), creation_date(lock.creation_date) {};

std::chrono::seconds EntryLock::ExpiresAt() const
{
    return std::chrono::seconds{expires_at};
}

std::chrono::seconds EntryLock::CreationDate() const
{
    return std::chrono::seconds{creation_date};
}

/******************/
/*  struct Entry  */
/******************/
Service::Entry::~Entry()
{
    if (lock != nullptr)
    {
        for (auto& pair : *lock)
            delete pair.second;
        lock->clear();
        delete lock;
        lock = nullptr;
    }

    if (children != nullptr)
    {
        for (Entry* it : *children)
            delete it;
        delete children;
        children = nullptr;
    }
}

/*******************/
/*  class Service  */
/*******************/
Service& Service::GetInstance()
{
    static Service instance{};
    return instance;
}

bool Service::Lock(const fs::path& path, const EntryLock& lock)
{
    std::list<std::string> cwd;
    Entry* entry = root_;
    std::string part_str;

    for (const fs::path& part : path)
    {
        part_str = part.string();

        if (part_str == "/")
        {
            entry = root_;
            cwd.clear();
            cwd.push_back("/");
            continue;
        }

        if (part_str == ".")
        {
            continue;
        }

        if (part_str == "..")
        {
            if (entry->parent == nullptr)
            {
                throw std::runtime_error("Illegial path.");
            }
            entry = entry->parent;
            cwd.pop_back();
            continue;
        }

        EntryListT* sub_entry = entry->children;
        if (sub_entry == nullptr)
        {
            sub_entry = new EntryListT();
            entry->children = sub_entry;
        }

        auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
        if (it == sub_entry->end())
        {
            auto* new_entry = new Entry();
            new_entry->name = part_str;
            for (const std::string& str : cwd)
            {
                new_entry->path /= str;
            }
            new_entry->path /= part_str;
            new_entry->parent = entry;
            sub_entry->push_back(new_entry);

            entry = new_entry;
            cwd.push_back(part_str);
            continue;
        }

        entry = *it;
        cwd.push_back(part_str);
    }

    if (entry->lock == nullptr)
    {
        entry->lock = new LockListT{{std::string{lock.user}, new EntryLock(lock)}};
        return true;
    }

    if (entry->lock->contains(lock.user))
    {
        return false;
    }

    entry->lock->insert({std::string{lock.user}, new EntryLock(lock)});
    return true;
}

bool Service::Lock(const fs::path& path, EntryLock&& lock)
{
    std::list<std::string> cwd;
    Entry* entry = root_;
    std::string part_str;

    for (const fs::path& part : path)
    {
        part_str = part.string();

        if (part_str == "/")
        {
            entry = root_;
            cwd.clear();
            cwd.push_back("/");
            continue;
        }

        if (part_str == ".")
        {
            continue;
        }

        if (part_str == "..")
        {
            if (entry->parent == nullptr)
            {
                throw std::runtime_error("Illegial path.");
            }
            entry = entry->parent;
            cwd.pop_back();
            continue;
        }

        EntryListT* sub_entry = entry->children;
        if (sub_entry == nullptr)
        {
            sub_entry = new EntryListT();
            entry->children = sub_entry;
        }

        auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
        if (it == sub_entry->end())
        {
            auto* new_entry = new Entry();
            new_entry->name = part_str;
            for (const std::string& str : cwd)
            {
                new_entry->path /= str;
            }
            new_entry->path /= part_str;
            new_entry->parent = entry;
            sub_entry->push_back(new_entry);

            entry = new_entry;
            cwd.push_back(part_str);
            continue;
        }

        entry = *it;
        cwd.push_back(part_str);
    }

    if (entry->lock == nullptr)
    {
        entry->lock = new LockListT{{std::string{lock.user}, new EntryLock(std::forward<EntryLock&&>(lock))}};
        return true;
    }

    if (entry->lock->contains(lock.user))
    {
        return false;
    }

    entry->lock->insert({std::string{lock.user}, new EntryLock(std::forward<EntryLock&&>(lock))});
    return true;
}

bool Service::Unlock(const fs::path& path, const std::string& user)
{
    Entry* entry = FindEntry(path);
    if (entry == nullptr)
    {
        return false;
    }

    if (entry->lock == nullptr) // lazy clean
    {
        delete entry;
        return false;
    }

    if (!entry->lock->contains(user))
    {
        return false;
    }

    entry->lock->erase(user);
    if (entry->lock->empty()) // lazy clean
        delete entry;
    return true;
}

const EntryLock* Service::GetLock(const fs::path& path, const std::string& user)
{
    Entry* entry = FindLock(path);
    assert(entry->lock != nullptr);

    if (entry == nullptr)
    {
        return nullptr;
    }

    auto it = entry->lock->find(user);
    if (it == entry->lock->end())
    {
        return nullptr;
    }

    return (*it).second;
}

const Service::LockListT* Service::GetAllLock(const fs::path& path)
{
    Entry* entry = FindLock(path);
    if (entry == nullptr || entry->lock == nullptr)
    {
        return nullptr;
    }

    return entry->lock;
}

bool Service::IsLocked(const fs::path& path, bool by_parent)
{
    Entry* entry = FindLock(path, by_parent);
    if (entry == nullptr || entry->lock->empty())
    {
        return false;
    }

    if (entry->path == path)
    {
        return true;
    }

    short diff_depth = 0;
    auto it1 = entry->path.begin();
    auto it2 = path.begin();
    while (it1 != entry->path.end() && it2 != path.end())
    {
        ++it1;
        ++it2;
    }

    if (it1 == entry->path.end())
    {
        while (it2 != path.end())
        {
            ++it2;
            diff_depth += 1;
        }
    }
    else
    {
        while (it1 != entry->path.end())
        {
            ++it1;
            diff_depth += 1;
        }
    }

    auto now_sec = utils::get_timestamp<std::chrono::seconds>().count();
    short max_depth = std::numeric_limits<short>::min();

    // list for expired lock
    std::vector<std::string> expired_lock;
    for (const auto& it : *(entry->lock))
    {
        // if the lock has expired, mark it
        if (it.second->expires_at < now_sec)
        {
            expired_lock.push_back(it.first);
            continue;
        }
        // considering that there will be multiple locks, we will use the lock with the highest depth value as the reference
        max_depth = std::max(max_depth, it.second->depth);
    }

    // clean up locks marked as expired
    for (const auto& k : expired_lock)
    {
        entry->lock->erase(k);
    }

    // are all the locks expired?
    if (entry->lock->empty())
    {
        return false;
    }

    return max_depth >= diff_depth;
}

bool Service::HoldingExclusiveLock(const fs::path& path)
{
    const auto* all_lock = GetAllLock(path);
    if (all_lock == nullptr)
    {
        return false;
    }

    for (const auto& item : *all_lock)
    {
        if (item.second->scope == FileLock::LockScope::EXCLUSIVE)
        {
            return true;
        }
    }

    return false;
}

Service::Service()
{
    root_ = new Entry();
}

Service::~Service()
{
    if (root_ == nullptr)
    {
        return;
    }

    delete root_;
    root_ = nullptr;
}

Service::Entry* Service::FindEntry(const fs::path& path)
{
    Entry* entry = root_;

    std::string part_str;
    for (const fs::path& part : path)
    {
        part_str = part.string();
        if (part_str == "/")
        {
            continue;
        }

        const std::list<Entry*>* sub_entry = entry->children;
        if (sub_entry == nullptr)
        {
            return nullptr;
        }

        auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
        if (it == sub_entry->end())
        {
            return nullptr;
        }

        entry = *it;
    }

    return entry;
}

Service::Entry* Service::FindLock(const fs::path& path, bool by_parent)
{
    Entry* entry = root_;

    std::string part_str;
    for (const fs::path& part : path)
    {
        if (by_parent && entry->lock != nullptr)
        {
            return entry;
        }

        part_str = part.string();
        if (part_str == "/")
        {
            continue;
        }

        auto* sub_entry = entry->children;
        if (sub_entry == nullptr)
        {
            return nullptr;
        }

        auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
        if (it == sub_entry->end())
        {
            return nullptr;
        }

        entry = *it;
    }

    // entry is not nullptr forever.
    if (entry->lock == nullptr)
    {
        return nullptr;
    }

    return entry;
}

} // namespace FileLock
