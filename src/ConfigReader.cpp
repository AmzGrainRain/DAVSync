#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <inih/cpp/INIReader.h>

#include "ConfigReader.h"
#include "utils/path.h"

static inline void user_parse(std::unordered_map<std::string, std::string>& users, const std::string_view& str)
{
    auto pos = str.find_last_of(':');
    if (pos == std::string_view::npos)
    {
        return;
    }

    users.emplace(std::string{str.substr(0, pos)}, std::string{str.substr(pos + 1)});
}

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
        long http_port = reader.GetInteger("http", "port", 8110);
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

    // === https =============================================================================
    https_enable_ = reader.GetBoolean("https", "enable", "false");
    {
        long https_port = reader.GetInteger("https", "port", 8111);
        assert(std::in_range<uint16_t>(https_port) && "[https.port] Must be within the range of [0-65535]");
        https_port_ = static_cast<uint16_t>(https_port);
    }
    https_only_ = reader.GetBoolean("https", "https-only", true);
    ssl_cert_ = reader.GetString("ssl", "cert", "");
    ssl_key_ = reader.GetString("ssl", "key", "");
    if (https_enable_)
    {
        assert(std::filesystem::is_regular_file(ssl_cert_) && "[ssl.cert] Not exist");
        assert(std::filesystem::is_regular_file(ssl_key_) && "[ssl.key] Not exist");
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

    webdav_realm_ = reader.GetString("webdav", "realm", "WebDavRealm");
    assert(!webdav_realm_.empty() && "[webdav.realm] Cannot be empty");

    webdav_verification_ = reader.GetString("wevdav", "verification", "none");
    assert((webdav_verification_ == "basic" || webdav_verification_ == "digest" || webdav_verification_ == "none") &&
           "[webdav.verification] Must be one of them [basic|digest|none]");

    webdav_reset_lock_ = reader.GetBoolean("webdav", "reset-lock", true);

    // parse user list
    std::string users_str = reader.GetString("webdav", "users", "");
    std::string_view users_view = users_str;
    for (size_t i = 0, j = 0; i < users_view.size(); ++i)
    {
        if (users_view[i] == ',')
        {
            if (j >= i)
            {
                continue;
            }
            user_parse(webdav_user_, users_view.substr(j, i));
            ++i;
            j = i;
        }
    }
    // =======================================================================================

    // === redis =============================================================================
    redis_host_ = reader.GetString("redis", "host", "localhost");

    {
        long redis_port = reader.GetInteger("redis", "port", 6379);
        assert(std::in_range<uint16_t>(redis_port) && "redis port number out of range");
        redis_port_ = static_cast<uint16_t>(redis_port);
    }

    redis_username_ = reader.GetString("redis", "username", "");
    assert(!redis_username_.empty() && "[redis.username] Cannot be empty");

    redis_password_ = reader.GetString("redis", "password", "");
    assert(!redis_password_.empty() && "[redis.password] Cannot be empty");
    // =======================================================================================

    // === engine ============================================================================
    std::unordered_set<std::string> engine_list{"memory", "sqlite", "redis"};

    etag_engine_ = reader.GetString("engine", "etag", "sqlite");
    assert(engine_list.contains(etag_engine_) && "[engine.etag] Must be one of them [memory|sqlite|redis]");

    lock_engine_ = reader.GetString("engine", "lock", "memory");
    assert(engine_list.contains(lock_engine_) && "[engine.lock] Must be one of them [memory|sqlite|redis]");

    prop_engine_ = reader.GetString("engine", "prop", "sqlite");
    assert(engine_list.contains(prop_engine_) && "[engine.prop] Must be one of them [memory|sqlite|redis]");
    // =======================================================================================

    // === cache =-===========================================================================
    sqlite_db_ = reader.GetString("cache", "sqlite-db", "./data.db");
    etag_data_ = reader.GetString("cache", "etag-data", "./etag.dat");
    lock_data_ = reader.GetString("cache", "lock-data", "./lock.dat");
    prop_data_ = reader.GetString("cache", "prop-data", "./prop.dat");
    // =======================================================================================
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

bool ConfigReader::GetHttpsEnabled() const noexcept
{
    return https_enable_;
}

uint16_t ConfigReader::GetHttpsPort() const noexcept
{
    return https_port_;
}

bool ConfigReader::GetHttpsOnly() const noexcept
{
    return https_only_;
}

const std::filesystem::path& ConfigReader::GetSSLCertPath() const noexcept
{
    return ssl_cert_;
}

const std::filesystem::path& ConfigReader::GetSSLKeyPath() const noexcept
{
    return ssl_key_;
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

int8_t ConfigReader::GetWebDavMaxRecurseDepth() const noexcept
{
    return webdav_max_recurse_depth_;
}

const std::string& ConfigReader::GetWebDavRealm() const noexcept
{
    return webdav_realm_;
}

const std::string& ConfigReader::GetWebDavVerification() const noexcept
{
    return webdav_verification_;
}

auto ConfigReader::GetWebDavUser() const noexcept -> const std::unordered_map<std::string, std::string>&
{
    return webdav_user_;
}

bool ConfigReader::GetWebDavResetLock() const noexcept
{
    return webdav_reset_lock_;
}

const std::string& ConfigReader::GetRedisHost() const noexcept
{
    return redis_host_;
}

uint16_t ConfigReader::GetRedisPort() const noexcept
{
    return static_cast<uint16_t>(redis_port_);
}

const std::string& ConfigReader::GetRedisUserName() const noexcept
{
    return redis_username_;
}

const std::string& ConfigReader::GetRedisPassword() const noexcept
{
    return redis_password_;
}

const std::string& ConfigReader::GetETagEngine() const noexcept
{
    return etag_engine_;
}

const std::string& ConfigReader::GetLockEngine() const noexcept
{
    return lock_engine_;
}

const std::string& ConfigReader::GetPropEngine() const noexcept
{
    return prop_engine_;
}

const std::filesystem::path& ConfigReader::GetSQLiteDB() const noexcept
{
    return sqlite_db_;
}

const std::filesystem::path& ConfigReader::GetETagData() const noexcept
{
    return etag_data_;
}

const std::filesystem::path& ConfigReader::GetLockData() const noexcept
{
    return lock_data_;
}

const std::filesystem::path& ConfigReader::GetPropData() const noexcept
{
    return prop_data_;
}
