#include <exception>
#include <cassert>
#include <iostream>
#include <ostream>
#include "config_reader.h"

int main(int argc, const char* argv[])
{
    try
    {
        const auto& conf = ConfigReader::GetInstance();

        // std::cout << config.GetHttpHost() << std::endl;
        // std::cout << config.GetHttpPort() << std::endl;
        // std::cout << (int)config.GetHttpMaxThread() << std::endl;
        // std::cout << config.GetRedisHost() << std::endl;
        // std::cout << (int)config.GetRedisPort() << std::endl;
        // std::cout << config.GetRedisAuth() << std::endl;
        // std::cout << config.GetUserAccount() << std::endl;
        // std::cout << config.GetUserPassword() << std::endl;

        assert(config.GetHttpHost() == "0.0.0.0");

        assert(config.GetHttpPort() == 8111);

        assert(config.GetHttpMaxThread() == 12);

        assert(config.GetRedisHost() == "localhost");

        assert(config.GetRedisPort() == 6379);

        assert(config.GetRedisAuth() == "");

        assert(config.GetUserAccount() == "test");

        assert(config.GetUserPassword() == "passw0rd");
    }
    catch (const std::exception& err)
    {
        std::cout << err.what() << std::endl;
        return -1;
    }

    return 0;
}
