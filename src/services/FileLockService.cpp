#include "FileLockService.h"
#include "utils.h"
#include <cassert>
#include <chrono>
#include <filesystem>

namespace FileLock
{

/*****************/
/*  struct Lock  */
/*****************/
EntryLock::EntryLock(const EntryLock& lock)
    : token(lock.token), depth(lock.depth), scope(lock.scope), type(lock.type), expires_at(lock.expires_at), creation_date(lock.creation_date) {};

EntryLock::EntryLock(EntryLock&& lock)
    : token(std::move(lock.token)), depth(lock.depth), scope(lock.scope), type(lock.type), expires_at(lock.expires_at),
      creation_date(lock.creation_date) {};

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
        delete lock;
        lock = nullptr;
    }

    if (children == nullptr)
    {
        return;
    }

    for (Entry* it : *children)
    {
        delete it;
    }
    delete children;
    children = nullptr;
}

/*******************/
/*  class Service  */
/*******************/
Service& Service::GetInstance()
{
    static Service instance{};
    return instance;
}

void Service::Lock(const std::filesystem::path& path, const EntryLock& lock)
{
    std::list<std::string> cwd;
    Entry* entry = root_;
    std::string part_str;

    for (const std::filesystem::path& part : path)
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

        std::list<Entry*>* sub_entry = entry->children;
        if (sub_entry == nullptr)
        {
            entry->children = new std::list<Entry*>();
            sub_entry = entry->children;
        }

        auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
        if (it == sub_entry->end())
        {
            auto new_entry = new Entry{};
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

    entry->lock = new EntryLock(lock);
}

bool Service::Unlock(const std::filesystem::path& path)
{
    Entry* entry = FindEntry(path);
    if (entry == nullptr)
    {
        return false;
    }

    delete entry->lock;
    entry->lock = nullptr;
    return true;
}

const EntryLock* Service::GetLock(const std::filesystem::path& path)
{
    const Entry* entry = FindLock(path);
    if (entry == nullptr)
    {
        return nullptr;
    }

    return entry->lock;
}

bool Service::IsLocked(const std::filesystem::path& path)
{
    Entry* entry = FindLock(path);
    if (entry == nullptr || entry->lock == nullptr)
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
    if (entry->lock->expires_at < now_sec)
    {
        delete entry->lock;
        entry->lock = nullptr;
        return false;
    }

    return entry->lock->depth >= diff_depth;
}

bool Service::LockExists(const std::filesystem::path& path)
{
    return FindEntry(path) != nullptr;
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

Service::Entry* Service::FindEntry(const std::filesystem::path& path)
{
    Entry* entry = root_;

    std::string part_str;
    for (const std::filesystem::path& part : path)
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

Service::Entry* Service::FindLock(const std::filesystem::path& path)
{
    Entry* entry = root_;

    std::string part_str;
    for (const std::filesystem::path& part : path)
    {
        if (entry->lock != nullptr)
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

    return entry;
}

} // namespace FileLock
