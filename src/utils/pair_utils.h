#pragma once

#include <string>
#include <vector>

namespace utils::pair
{

std::string pairs_stringify(const std::vector<std::pair<std::string, std::string>>& pairs,
                            const std::string_view& kv_separator, const std::string_view& separator);

}
