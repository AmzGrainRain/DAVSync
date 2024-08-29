#include <cassert>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <variant>

#include <inih/cpp/INIReader.h>

#include "config_reader.h"
#include "utils/path.h"

inline const void ConfigReader::WriteDefaultConfigFile(const std::filesystem::path& file)
{
    std::cout << "unable to read configuration file, creating ..." << std::endl;

    std::ofstream ofs(file, std::ios::out);
    if (!ofs.is_open())
    {
        std::cout << "unable to write default configuration file, please check program permissions and try again."
                  << std::endl;
        return;
    }

    const std::string default_config = "[http]\n"
                                       "host = 127.0.0.1\n"
                                       "address = 0.0.0.0\n"
                                       "port = 8111\n"
                                       "buffer_size = 1024\n"
                                       "max_thread = 0\n"

                                       "[ssl]\n"
                                       "enable = false\n"
                                       "cert =./full_chain.pem\n"
                                       "key =./key.pem\n"

                                       "[webdav]\n"
                                       "prefix = /webdav\n"
                                       "root-path = ./data\n"
                                       "verification = none\n"
                                       "realm = WebDavRealm\n"
                                       "max-recurse-depth = 4\n"

                                       "[redis]\n"
                                       "host = localhost\n"
                                       "port = 6379\n"
                                       "auth = \n"

                                       "[user]\n"
                                       "account =\n"
                                       "password =\n";
    ofs.write(default_config.data(), default_config.size());
    ofs.flush();
    ofs.close();
    std::cout << "done." << std::endl;
}

const ConfigReader& ConfigReader::GetInstance()
{
    static ConfigReader instance;

    return instance;
}

ConfigReader::ConfigReader(const std::filesystem::path& path)
{
    std::string path_string = utils::path::to_string(path);
    if (!std::filesystem::exists(path_string))
    {
        WriteDefaultConfigFile(path);
    }

    INIReader reader(path_string);
    assert(reader.ParseError() == 0 && "failed to parsing settings.ini");

    cwd_ = std::filesystem::current_path();

    /* HTTP */
    http_host_ = reader.GetString("http", "host", "127.0.0.1");
    assert(!http_host_.empty() && "illegal http host");

    http_address_ = reader.GetString("http", "address", "0.0.0.0");
    assert(!http_address_.empty() && "illegal http address");

    {
        long http_port = reader.GetInteger("http", "port", 8111);
        assert(std::in_range<uint16_t>(http_port) && "port number out of range");
        http_port_ = static_cast<uint16_t>(http_port);
    }

    {
        long http_max_thread = reader.GetInteger("http", "max_thread", 0);
        assert(std::in_range<uint16_t>(http_max_thread) && "thread number out of range [impossible?]");
        http_max_thread_ = static_cast<uint16_t>(http_max_thread);
    }

    http_buffer_size_ = static_cast<size_t>(reader.GetUnsigned64("http", "buffer_size", 1024));
    assert((http_buffer_size_ >= 1024) && "the buffer size shoule be greater than 1024");

    /* SSL */
    {
        auto ssl_enable = reader.GetString("ssl", "enable", "no");
        assert((ssl_enable == "true" || ssl_enable == "false") && "ssl_enable must be one of them [true | false]");
        ssl_enable_ = ssl_enable == "true" ? true : false;
    }
    ssl_cert_path_ = reader.GetString("ssl", "cert", "");
    ssl_key_path_ = reader.GetString("ssl", "key", "");
    if (ssl_enable_)
    {
        assert(std::filesystem::is_regular_file(ssl_cert_path_) && "SSL certificate does not exist");
        assert(std::filesystem::is_regular_file(ssl_key_path_) && "SSL private key does not exist");
    }

    // webdav
    webdav_prefix_ = reader.GetString("webdav", "prefix", "/webdav");
    assert(!webdav_prefix_.empty() && "the webdav prefix cannot be empty");
    {
        char c = webdav_prefix_.back();
        assert((c >= 65 && c <= 90) || (c >= 97 && c <= 122) && "the prefix should end with the [a-Z] character");
    }
    webdav_raw_prefix_ = webdav_prefix_ + "/(.*)";

    std::filesystem::path data_path = reader.GetString("webdav", "root-path", "data");
    assert((std::filesystem::exists(data_path) && std::filesystem::is_directory(data_path)) &&
           "webdav root path is not available");

    if (data_path.is_absolute())
    {
        webdav_absolute_data_path_ = std::move(data_path);
        webdav_relative_data_path_ = std::filesystem::relative(webdav_absolute_data_path_, cwd_).lexically_normal();
    }
    else
    {
        webdav_relative_data_path_ = std::move(data_path);
        webdav_absolute_data_path_ = (cwd_ / webdav_relative_data_path_).lexically_normal();
    }

    {
        long webdav_max_recurse_depth = reader.GetInteger("webdav", "max-recurse-depth", 4);
        assert(std::in_range<int8_t>(webdav_max_recurse_depth) &&
               "the maximum recursive depth of webdav exceeds the range");
        webdav_max_recurse_depth_ = static_cast<int8_t>(webdav_max_recurse_depth);
    }

    webdav_verification_ = reader.GetString("wevdav", "verification", "none");
    assert((webdav_verification_ == "basic" || webdav_verification_ == "digest" || webdav_verification_ == "none") &&
           "webdav verification must be one of them [basic|digest|none]");

    webdav_realm_ = reader.GetString("webdav", "realm", "WebDavRealm");
    assert(!webdav_realm_.empty() && "the webdav realm cannot be empty");

    // redis
    redis_host_ = reader.GetString("redis", "host", "localhost");

    {
        long redis_port = reader.GetInteger("redis", "port", 6379);
        assert(std::in_range<uint16_t>(redis_port) && "redis port number out of range");
        redis_port_ = static_cast<uint16_t>(redis_port);
        if (redis_port_ == 0)
        {
            redis_port_ = 6379;
        }
    }

    redis_auth_ = reader.GetString("redis", "auth", "");

    // user
    user_account_ = reader.GetString("user", "account", "");
    user_passwd_ = reader.GetString("user", "password", "");
    if (webdav_verification_ != "none")
    {
        assert(!user_account_.empty() && "the user account cannot be empty");
        assert((user_account_.size() >= 8) && "the user password length must be greater than 8 digits");
    }
}

const std::filesystem::path& ConfigReader::GetCWD() const noexcept
{
    return cwd_;
}

const std::string& ConfigReader::GetHttpHost() const noexcept
{
    return http_host_;
}

const std::string& ConfigReader::GetHttpAddress() const noexcept
{
    return http_address_;
}

uint16_t ConfigReader::GetHttpPort() const noexcept
{
    return static_cast<uint16_t>(http_port_);
}

uint16_t ConfigReader::GetHttpMaxThread() const noexcept
{
    return http_max_thread_;
}

size_t ConfigReader::GetHttpBufferSize() const noexcept
{
    return http_buffer_size_;
}

bool ConfigReader::GetSSLEnabled() const noexcept
{
    return ssl_enable_;
}

const std::filesystem::path& ConfigReader::GetSSLCertPath() const noexcept
{
    return ssl_cert_path_;
}

const std::filesystem::path& ConfigReader::GetSSLKeyPath() const noexcept
{
    return ssl_key_path_;
}

const std::string& ConfigReader::GetWebDavPrefix() const noexcept
{
    return webdav_prefix_;
}

const std::string& ConfigReader::GetWebDavRawPrefix() const noexcept
{
    return webdav_raw_prefix_;
}

const std::filesystem::path& ConfigReader::GetWebDavRelativeDataPath() const noexcept
{
    return webdav_relative_data_path_;
}

const std::filesystem::path& ConfigReader::GetWebDavAbsoluteDataPath() const noexcept
{
    return webdav_absolute_data_path_;
}

const std::string& ConfigReader::GetWebDavVerification() const noexcept
{
    return webdav_verification_;
}

const std::string& ConfigReader::GetWebDavRealm() const noexcept
{
    return webdav_realm_;
}

int8_t ConfigReader::GetWebDavMaxRecurseDepth() const noexcept
{
    return webdav_max_recurse_depth_;
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
