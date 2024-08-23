#include <cstdint>
#include <exception>
#include <cassert>
#include <type_traits>
#include <string>

#include "config_reader.h"

int main(int argc, const char* argv[])
{
    try
    {
        bool same = false;
        ConfigReader config("./settings.ini");
        assert(config.GetHttpHost() == "0.0.0.0");
        same = std::is_same_v<decltype(config.GetHttpHost()), std::string>;
        assert(same);

        assert(config.GetHttpPort() == 80);
        same = std::is_same_v<decltype(config.GetHttpPort()), uint16_t>;
        assert(same);

        assert(config.GetRedisHost() == "localhost");
        same = std::is_same_v<decltype(config.GetRedisHost()), std::string>;
        assert(same);


        assert(config.GetRedisPort() == 6379);
        same = std::is_same_v<decltype(config.GetRedisPort()), uint16_t>;
        assert(same);


        assert(config.GetRedisAuth() == "");
        same = std::is_same_v<decltype(config.GetRedisAuth()), std::string>;
        assert(same);
    }
    catch (const std::exception& err)
    {
        return -1;
    }

    return 0;
}
