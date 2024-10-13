#include <algorithm>
#include <iostream>
#include <pugixml.hpp>
#include <vcruntime.h>

int main()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("D:\\Repos\\AmzGrainRain\\DAVSync\\tests\\lock.xml");
    if (!result)
        return -1;

    auto is_shared = doc.select_node("/D:lockinfo/D:lockscope/D:shared");
    auto is_exclusive = doc.select_node("/D:lockinfo/D:lockscope/D:exclusive");
    if (is_shared && !is_exclusive)
    {
        std::cout << "shared ";
    }
    else if (!is_shared && is_exclusive)
    {
        std::cout << "exclusive ";
    }
    else
    {
        std::cout << "none ";
    }

    auto is_read = doc.select_node("/D:lockinfo/D:locktype/D:read");
    auto is_write = doc.select_node("/D:lockinfo/D:locktype/D:write");
    if (is_read && !is_write)
    {
        std::cout << "read\n";
    }
    else if (!is_read && is_write)
    {
        std::cout << "write\n";
    }
    else
    {
        std::cout << "none\n";
    }

    auto desc = doc.select_node("/D:lockinfo/D:owner");
    std::string desc_str;
    if (desc)
    {
        for (auto n : desc.node().children())
        {
            desc_str += n.name();
            desc_str += ':';
            desc_str += n.text().as_string();
            desc_str += ',';
        }
        size_t end = desc_str.length() - 1;
        desc_str = desc_str.substr(0, std::max(static_cast<size_t>(0), end));
    }
    std::cout << desc_str << std::endl;

    return 0;
}
