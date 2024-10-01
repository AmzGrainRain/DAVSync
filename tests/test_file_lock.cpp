#include <algorithm>
#include <filesystem>
#include <list>
#include <iostream>
#include <stdexcept>

enum class EntryLockType
{
	NIL = 0,
	SHARED,
	EXCLUSIVE
};

struct Entry
{
	EntryLockType lock = EntryLockType::NIL;
	std::string name = "";
	std::filesystem::path path = "/";
	Entry* parent = nullptr;
	std::list<Entry*>* children = nullptr;

	~Entry()
	{
		if (children == nullptr)
		{
			return;
		}

		for (Entry* it : *children)
		{
			delete it;
		}
		children = nullptr;
	}
};

class LockService
{
public:
	static LockService& GetInstance()
	{
		static LockService instance{};
		return instance;
	}

	void Lock(const std::filesystem::path& path, EntryLockType lock_type)
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

		entry->lock = lock_type;
	}

	bool Unlock(const std::filesystem::path& path)
	{
		Entry* entry = FindEntry(path);
		if (entry == nullptr)
		{
			return false;
		}

		entry->lock = EntryLockType::NIL;
		return true;
	}

	EntryLockType GetLock(const std::filesystem::path& path)
	{
		Entry* entry = FindEntry(path);
		if (entry == nullptr)
		{
			return EntryLockType::NIL;
		}

		return entry->lock;
	}

	bool IsLocked(const std::filesystem::path& path)
	{
		Entry* entry = root_;

		std::string part_str;
		for (const std::filesystem::path& part : path)
		{
			if (entry->lock != EntryLockType::NIL)
			{
				return true;
			}

			part_str = part.string();
			if (part_str == "/")
			{
				continue;
			}

			auto* sub_entry = entry->children;
			if (sub_entry == nullptr)
			{
				return false;
			}

			auto it = std::find_if(sub_entry->begin(), sub_entry->end(), [&part_str](const Entry* elm) { return elm->name == part_str; });
			if (it == sub_entry->end())
			{
				return false;
			}

			entry = *it;
		}

		return entry->lock != EntryLockType::NIL;
	}

	bool LockExists(const std::filesystem::path& path)
	{
		return FindEntry(path) != nullptr;
	}

	~LockService()
	{
		if (root_ == nullptr)
		{
			return;
		}

		delete root_;
		root_ = nullptr;
	}

private:
	LockService()
	{
		root_ = new Entry();
	}

	Entry* FindEntry(const std::filesystem::path& path)
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

	Entry* root_ = nullptr;
};

int main()
{
	auto& lock = LockService::GetInstance();

	lock.Lock("/a/b", EntryLockType::EXCLUSIVE);
	std::cout << "/a/b -> LOCKED" << std::endl;


	if (lock.IsLocked("/a/b/c/d"))
	{
		std::cout << "/a/b/c/d -> LOCKED" << std::endl;
	}
	else
	{
		std::cout << "/a/b/c/d -> UNLOCKED" << std::endl;
	}


	switch (lock.GetLock("/a/b"))
	{
	case EntryLockType::NIL:
		std::cout << "/a/b -> UNLOCKED" << std::endl;
		break;
	case EntryLockType::SHARED:
		std::cout << "/a/b -> SHARED LOCK" << std::endl;
		break;
	case EntryLockType::EXCLUSIVE:
		std::cout << "/a/b -> EXCLUSIVE LOCK" << std::endl;
		break;
	}

	return 0;
}
