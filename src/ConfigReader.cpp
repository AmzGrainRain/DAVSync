#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

#include <inih/cpp/INIReader.h>
#include "ConfigReader.h"
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
                                       "max_thread = 0\n\n"

                                       "[ssl]\n"
                                       "enable = false\n"
                                       "cert =./full_chain.pem\n"
                                       "key =./key.pem\n\n"

                                       "[webdav]\n"
                                       "prefix = /webdav\n"
                                       "root-path = ./data\n"
                                       "verification = none\n"
                                       "realm = WebDavRealm\n"
                                       "max-recurse-depth = 4\n\n"

                                       "[sqlite]\n"
                                       "enable = true\n"
                                       "location = ./file_props.db\n\n"

                                       "[redis]\n"
                                       "enable = false\n"
                                       "host = localhost\n"
                                       "port = 6379\n"
                                       "auth = \n\n"

                                       "[user]\n"
                                       "account =\n"
                                       "password =\n\n";
    ofs.write(default_config.data(), default_config.size());
    ofs.flush();
    ofs.close();
    std::cout << "done." << std::endl;
}

const ConfigReader& ConfigReader::GetInstance()
{
    static ConfigReader instance{};
    return instance;
}

ConfigReader::ConfigReader(const std::filesystem::path& path)
{
    namespace fs = std::filesystem;

    std::string path_string = utils::path::to_string(path);
    if (!fs::exists(path_string))
    {
        WriteDefaultConfigFile(path);
    }

    INIReader reader(path_string);
    assert(reader.ParseError() == 0 && "Failed to parsing settings.ini");

    cwd_ = fs::current_path();

    // === http ==============================================================================
    http_host_ = reader.GetString("http", "host", "127.0.0.1");
    assert(!http_host_.empty() && "[http.host] Illegal host");

    http_address_ = reader.GetString("http", "address", "0.0.0.0");
    assert(!http_address_.empty() && "[http.address] Illegal address");

    {
        long http_port = reader.GetInteger("http", "port", 8111);
        assert(std::in_range<uint16_t>(http_port) && "[http.port] Must be within the range of [0-65535]");
        http_port_ = static_cast<uint16_t>(http_port);
    }

    http_buffer_size_ = static_cast<size_t>(reader.GetUnsigned64("http", "buffer-size", 1024));
    assert((http_buffer_size_ >= 1024) && "[http.buffer-size] Must be >= 1024");

    {
        long http_max_thread = reader.GetInteger("http", "max_thread", 0);
        assert(std::in_range<uint16_t>(http_max_thread) && "[http.max-thread] Must be within the range of [0-65535]");
        http_max_thread_ = static_cast<uint16_t>(http_max_thread);
        if (http_max_thread_ == 0)
        {
            http_max_thread_ = std::thread::hardware_concurrency();
        }
    }
    // =======================================================================================

    // === ssl ===============================================================================
    ssl_enable_ = reader.GetBoolean("ssl", "enable", "false");
    ssl_cert_path_ = reader.GetString("ssl", "cert", "");
    ssl_key_path_ = reader.GetString("ssl", "key", "");
    if (ssl_enable_)
    {
        assert(std::filesystem::is_regular_file(ssl_cert_path_) && "[ssl.cert] Not exist");
        assert(std::filesystem::is_regular_file(ssl_key_path_) && "[ssl.key] Not exist");
    }
    // =======================================================================================

    // === webdav ============================================================================
    webdav_prefix_ = reader.GetString("webdav", "prefix", "./webdav");
    assert(!webdav_prefix_.empty() && "[webdav.prefix] Cannot be empty");
    {
        char c = webdav_prefix_.back();
        assert((c >= 65 && c <= 90) || (c >= 97 && c <= 122) && "[webdav.prefix] Must end with the [a-Z] character");
    }

    webdav_route_prefix_ = webdav_prefix_ + "/(.*)";

    {
        fs::path data_path = reader.GetString("webdav", "data-path", "data");
        assert(std::filesystem::exists(data_path) && "[webdav.data-path] Not exist");
        assert(std::filesystem::is_directory(data_path) && "[webdav.data-path] Required directory");
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
    }

    {
        long webdav_max_recurse_depth = reader.GetInteger("webdav", "max-recurse-depth", 4);
        assert(std::in_range<int8_t>(webdav_max_recurse_depth) &&
               "[webdav.max-recurse-depth] Must be within the range of [0-255]");
        webdav_max_recurse_depth_ = static_cast<int8_t>(webdav_max_recurse_depth);
    }

    webdav_verification_ = reader.GetString("wevdav", "verification", "none");
    assert((webdav_verification_ == "basic" || webdav_verification_ == "digest" || webdav_verification_ == "none") &&
           "[webdav.verification] Must be one of them [basic|digest|none]");

    webdav_realm_ = reader.GetString("webdav", "realm", "WebDavRealm");
    assert(!webdav_realm_.empty() && "[webdav.realm] Cannot be empty");
    // =======================================================================================

    // === sqlite ============================================================================
    sqlite_enable_ = reader.GetBoolean("sqlite", "enable", true);
    sqlite_location_ = reader.GetString("sqlite", "location", "./file_index.db");
    // =======================================================================================

    // === redis =============================================================================
    redis_enable_ = reader.GetBoolean("redis", "enable", false);
    redis_host_ = reader.GetString("redis", "host", "localhost");

    {
        long redis_port = reader.GetInteger("redis", "port", 6379);
        assert(std::in_range<uint16_t>(redis_port) && "redis port number out of range");
        redis_port_ = static_cast<uint16_t>(redis_port);
    }

    redis_auth_ = reader.GetString("redis", "auth", "");
    // =======================================================================================

    // === user ==============================================================================
    user_account_ = reader.GetString("user", "account", "");
    user_passwd_ = reader.GetString("user", "password", "");
    if (webdav_verification_ != "none")
    {
        assert(!user_account_.empty() && "the user account cannot be empty");
        assert((user_account_.size() >= 8) && "the user password length must be greater than 8 digits");
    }
    // =======================================================================================

    /* check for database conflicts */
    if (!sqlite_enable_ && !redis_enable_)
    {
        std::cerr << "It seems that you have disabled both SQLite and Redis, SQLite will be prioritized here." << std::endl;
        sqlite_enable_ = true;
    }
    else if (sqlite_enable_ && redis_enable_)
    {
        sqlite_enable_ = false;
        std::cerr << "It seems that you have enabled both SQLite and Redis, Redis will be prioritized here." << std::endl;
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

const std::string& ConfigReader::GetWebDavRoutePrefix() const noexcept
{
    return webdav_route_prefix_;
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

bool ConfigReader::GetSQLiteEnable() const noexcept
{
    return sqlite_enable_;
}

 const std::filesystem::path& ConfigReader::GetSQLiteLocation() const noexcept
{
    return sqlite_location_;
}

bool ConfigReader::GetRedisEnable() const noexcept
{
    return redis_enable_;
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
