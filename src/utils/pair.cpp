#include "pair.h"

namespace utils::pair
{

std::string pairs_stringify(const std::vector<std::pair<std::string, std::string>>& pairs,
                            const std::string_view& kv_separator, const std::string_view& separator)
{
    std::string result{};
    for (const auto& [k, v] : pairs)
    {
        result += k;
        result += kv_separator;
        result += v;
        result += separator;
    }

    return result.substr(0, result.size() - separator.size());
}

} // namespace utils::pair
