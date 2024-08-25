#include "pair_utils.h"

namespace utils::pair
{

std::string pairs_stringify(const std::vector<std::pair<std::string, std::string>>& pairs,
                            const std::string_view& kv_separator, const std::string_view& separator)
{
    std::string result{};
    for (const auto& pair : pairs)
    {
        result += pair.first;
        result += kv_separator;
        result += pair.second;
        result += separator;
    }

    return result.substr(0, result.size() - separator.size());
}

} // namespace utils::pair
