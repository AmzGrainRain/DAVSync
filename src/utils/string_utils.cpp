#include "string_utils.h"

namespace utils::string
{

std::string u8str2str(const std::u8string& u8str)
{
    return std::string{u8str.begin(), u8str.end()};
}

std::string path2str(const std::filesystem::path& path)
{
    return u8str2str(path.u8string());
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

    size_t pos = str.find(separator);
    while (pos != std::string::npos)
    {
        std::string value = str.substr(pos, separator.size());
        trim(value);
        tokens.push_back(std::move(value));
    }

    tokens.push_back(str.substr(pos));
    return tokens;
}

auto split2pair(const std::string& str,
                const std::string_view& separator) -> std::optional<std::pair<std::string, std::string>>
{
    size_t pos = str.find(separator);
    if (pos == std::string::npos)
    {
        return std::nullopt;
    }

    return std::pair<std::string, std::string>{str.substr(0, pos), str.substr(pos + 1)};
}

} // namespace utils::string
