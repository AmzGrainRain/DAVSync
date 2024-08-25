#include "config_reader.h"
#include <inih/cpp/INIReader.h>
#include <cstdint>
#include <thread>
#include <exception>

#include "utils/string_utils.h"

const ConfigReader& ConfigReader::GetInstance()
{
    static ConfigReader instance;

    return instance;
}

ConfigReader::ConfigReader(const std::filesystem::path& path)
{
    std::string path_string = utils::string::path2str(path);
    INIReader reader(path_string);
    if (reader.ParseError() < 0)
    {
        path_string = "Unable to read '" + path_string + "'";
        throw std::exception(path_string.c_str());
    }

    // http
    http_host_ = reader.GetString("http", "address", "0.0.0.0");
    http_port_ = static_cast<uint16_t>(reader.GetInteger("http", "port", 8111));
    http_max_thread_ = static_cast<uint8_t>(reader.GetInteger("http", "max_thread", std::thread::hardware_concurrency()));
    if (http_max_thread_ <= 0)
    {
        http_max_thread_ = static_cast<uint8_t>(std::thread::hardware_concurrency());
    }

    // webdav
    webdav_prefix_ = reader.GetString("webdav", "prefix", "/webdav");
    webdav_verification_ = reader.GetString("wevdav", "verification", "Digest");
    webdav_realm_ = reader.GetString("webdav", "realm", "WebDavRealm");

    // redis
    redis_host_ = reader.GetString("redis", "host", "localhost");
    redis_port_ = static_cast<uint16_t>(reader.GetInteger("redis", "port", 6379));
    if (redis_port_ <= 0)
    {
        redis_port_ = 6379;
    }
    redis_auth_ = reader.GetString("redis", "auth", "");

    // user
    user_account_ = reader.GetString("user", "account", "");
    user_passwd_ = reader.GetString("user", "password", "");
}

const std::string& ConfigReader::GetHttpHost() const noexcept
{
    return http_host_;
}

uint16_t ConfigReader::GetHttpPort() const noexcept
{
    return static_cast<uint16_t>(http_port_);
}

uint8_t ConfigReader::GetHttpMaxThread() const noexcept
{
    return http_max_thread_;
}

const std::string& ConfigReader::GetWebDavPrefix() const noexcept
{
    return webdav_prefix_;
}

const std::string& ConfigReader::GetWebDavVerification() const noexcept
{
    return webdav_verification_;
}

const std::string& ConfigReader::GetWebDavRealm() const noexcept
{
    return webdav_realm_;
}

const std::string& ConfigReader::GetRedisHost() const noexcept
{
    return redis_host_;
}

uint16_t ConfigReader::GetRedisPort() const noexcept
{
    return static_cast<uint16_t>(redis_port_);
}

const std::string& ConfigReader::GetRedisAuth() const noexcept
{
    return redis_auth_;
}

const std::string& ConfigReader::GetUserAccount() const noexcept
{
    return user_account_;
}

const std::string& ConfigReader::GetUserPassword() const noexcept
{
    return user_passwd_;
}
