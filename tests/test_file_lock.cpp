#include <algorithm>
#include <cassert>
#include <filesystem>
#include <list>
#include <stdexcept>

template <class T>
concept IsTimeType = std::is_same_v<T, std::chrono::nanoseconds> || std::is_same_v<T, std::chrono::microseconds> ||
std::is_same_v<T, std::chrono::milliseconds> || std::is_same_v<T, std::chrono::seconds>;
template <IsTimeType T = std::chrono::nanoseconds> T get_timestamp()
{
	using namespace std::chrono;
	return duration_cast<T>(system_clock::now().time_since_epoch());
}

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
	long long creation_date = get_timestamp<std::chrono::seconds>().count();

	EntryLock() = default;

	EntryLock(const EntryLock& lock) noexcept
		: token(lock.token), depth(lock.depth), scope(lock.scope), type(lock.type), expires_at(lock.expires_at), creation_date(lock.creation_date) {};

	EntryLock(EntryLock&& lock) noexcept
		: token(std::move(lock.token)), depth(lock.depth), scope(lock.scope), type(lock.type), expires_at(lock.expires_at),
		creation_date(lock.creation_date) {};

	~EntryLock() = default;

	std::chrono::seconds ExpiresAt() const
	{
		return std::chrono::seconds{ expires_at };
	}

	std::chrono::seconds CreationDate() const
	{
		return std::chrono::seconds{ creation_date };
	}
};

class Service
{
public:
	Service()
	{
		root_ = new Entry();
	}

	~Service()
	{
		if (root_ == nullptr)
		{
			return;
		}

		delete root_;
		root_ = nullptr;
	}

	void Lock(const std::filesystem::path& path, const EntryLock& lock)
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

	bool Unlock(const std::filesystem::path& path)
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

	const EntryLock* GetLock(const std::filesystem::path& path)
	{
		const Entry* entry = FindLock(path);
		if (entry == nullptr)
		{
			return nullptr;
		}

		return entry->lock;
	}

	bool IsLocked(const std::filesystem::path& path)
	{
		const Entry* entry = FindLock(path);

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

		return entry->lock->depth >= diff_depth;
	}

	bool LockExists(const std::filesystem::path& path)
	{
		return FindEntry(path) != nullptr;
	}

private:
	struct Entry
	{
		std::string name = "";
		std::filesystem::path path = "/";

		EntryLock* lock = nullptr;
		Entry* parent = nullptr;
		std::list<Entry*>* children = nullptr;

		Entry() = default;
		~Entry()
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
	};

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

	const Service::Entry* FindLock(const std::filesystem::path& path)
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

	Entry* root_ = nullptr;
};

void test()
{
	Service service{};

	{ // set lock
		EntryLock lock;
		lock.depth = 3;
		lock.token = "123";
		service.Lock("/a/b", std::move(lock));
	}

	{ // locked
		assert(service.IsLocked("/a/b"));
		assert(service.IsLocked("/a/b/c/d/e"));
		assert(!service.IsLocked("/a/b/c/d/e/f"));
	}

	{ // get lock
		const EntryLock* lock = service.GetLock("/a/b");
		assert(lock != nullptr);
	}

	{ // unlock
		assert(service.Unlock("/a/b"));
	}

	{ // locked
		assert(!service.IsLocked("/a/b"));
		assert(!service.IsLocked("/a/b/c/d/e"));
		assert(!service.IsLocked("/a/b/c/d/e/f"));
	}
}

int main()
{
	test();

	return 0;
}
