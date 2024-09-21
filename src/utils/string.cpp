#include "string.h"

namespace utils::string
{

std::string u8str2str(const std::u8string& u8str)
{
    return std::string{u8str.begin(), u8str.end()};
}

void replace(std::string& str, const std::string& search, const std::string_view& replace_with)
{
    size_t pos = str.find(search);
    if (pos == std::string::npos)
    {
        return;
    }

    str.replace(pos, search.size(), replace_with);
}

void trim(std::string& str)
{
    if (str.empty())
    {
        return;
    }

    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos || end == std::string::npos)
    {
        return;
    }

    str = str.substr(start, (end - start + 1));
}

std::vector<std::string> split(const std::string& str, const std::string_view& separator)
{
    std::vector<std::string> tokens;
    size_t str_len = str.length();
    size_t last_pos = 0, pos;
    while ((pos = str.find(separator, last_pos)) != std::string::npos)
    {
        tokens.emplace_back(str.substr(last_pos, pos - last_pos));
        last_pos = pos + 1;
        if (last_pos >= str_len)
        {
            break;
        }
    }

    if (last_pos < str_len)
    {
        tokens.emplace_back(str.substr(last_pos));
    }

    return tokens;
}

auto split2pair(const std::string& str, char separator) -> std::pair<std::string, std::string>
{
    size_t pos = str.find(separator);
    if (pos == std::string::npos)
    {
        return {std::string{str}, {}};
    }

    auto k = str.substr(0, pos);
    auto v = str.substr(pos + 1);
    trim(k);
    trim(v);
    return std::pair{std::move(k), std::move(v)};
}

auto split2pair(const std::string_view& str, char separator) -> std::pair<std::string, std::string>
{
    size_t pos = str.find(separator);
    if (pos == std::string::npos)
    {
        return {std::string{str}, {}};
    }

    auto k = str.substr(0, pos);
    auto v = str.substr(pos + 1);
    return std::pair{std::string(k), std::string(v)};
}

} // namespace utils::string
