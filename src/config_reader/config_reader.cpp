#include <cstdint>
#include <thread>
#include <exception>

#include "inih/cpp/INIReader.h"

#include "../utils.h"
#include "config_reader.h"

ConfigReader::ConfigReader(const std::filesystem::path& path)
{
    std::string path_string = path2str(path);
    INIReader reader(path_string);
    if (reader.ParseError() < 0)
    {
        path_string = " '" + path_string + "'";
        throw std::exception(path_string.c_str());
    }

    // http
    this->http_host_ = reader.GetString("http", "host", "0.0.0.0");
    this->http_port_ = static_cast<uint16_t>(reader.GetInteger("http", "port", 80));
    this->http_max_thread_ = static_cast<uint8_t>(reader.GetInteger("http", "max_thread", std::thread::hardware_concurrency()));

    // redis
    this->redis_host_ = reader.GetString("redis", "host", "0.0.0.0");
    this->redis_port_ = static_cast<uint16_t>(reader.GetInteger("redis", "host", 6379));
    this->redis_auth_ = reader.GetString("redis", "auth", "");

    // auth
    this->user_name_ = reader.GetString("auth", "user", "");
    this->user_name_ = reader.GetString("auth", "password", "");
}

const std::string& ConfigReader::GetHttpHost() const noexcept
{
    return http_host_;
}

uint16_t ConfigReader::GetHttpPort() const noexcept
{
    return http_port_;
}

uint8_t ConfigReader::GetHttpMaxThread() const noexcept
{
    return http_max_thread_;
}

const std::string& ConfigReader::GetRedisHost() const noexcept
{
    return redis_host_;
}

uint16_t ConfigReader::GetRedisPort() const noexcept
{
    return redis_port_;
}

const std::string& ConfigReader::GetRedisAuth() const noexcept
{
    return redis_auth_;
}

const std::string& ConfigReader::GetUserName() const noexcept
{
    return user_name_;
}

const std::string& ConfigReader::GetUserPassword() const noexcept
{
    return user_passwd_;
}
