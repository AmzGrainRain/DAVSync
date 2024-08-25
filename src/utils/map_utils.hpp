#pragma once

#include <map>
#include <unordered_map>
#include <vector>

namespace utils::map
{

template <class MapKeyType, class MapValueType>
bool all_exist(const std::map<MapKeyType, MapValueType>& map, const std::vector<MapValueType>& keys)
{
    for (const auto& key : keys)
    {
        if (!map.contains(key))
        {
            return false;
        }
    }

    return true;
}

template <class MapKeyType, class MapValueType>
bool all_exist(const std::unordered_map<MapKeyType, MapValueType>& map, const std::vector<MapValueType>& keys)
{
    for (const auto& key : keys)
    {
        if (!map.contains(key))
        {
            return false;
        }
    }

    return true;
}

} // namespace utils::map
