#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>

int main()
{
    std::unordered_map<std::string, std::string> map;
    map.insert({"1", "123123"});

    std::unordered_map<std::string, std::string>::iterator a = map.begin();

    return 0;
}
