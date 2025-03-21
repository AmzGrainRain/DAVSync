#include "path.h"

#include <cstdint>
#include <filesystem>

namespace utils::path
{

std::string to_string(const std::filesystem::path& path) noexcept
{
    auto u8str = path.u8string();
    return std::string{u8str.begin(), u8str.end()};
}

std::string with_separator(const std::filesystem::path& path, const bool is_dir, const char separator, uint16_t skip) noexcept
{

    auto it = path.begin();

    // skip
    while (skip > 0 && it != path.end())
    {
        ++it;
        --skip;
    }
    if (it == path.end())
    {
        return {};
    }

    // concat
    std::u8string u8str = it->u8string();
    std::string ret{u8str.begin(), u8str.end()};
    // the default value of u8str has already used it, so here ++it
    for (++it; it != path.end(); ++it)
    {
        u8str = it->u8string();
        ret += separator;
        ret += {u8str.begin(), u8str.end()};
    }

    // append
    if (is_dir)
    {
        ret += separator;
    }

    return ret;
}

} // namespace utils::path
