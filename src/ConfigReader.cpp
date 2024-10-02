#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <inih/cpp/INIReader.h>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils/path.h"
#include "utils/string.h"

static inline void ParseHttpConfig(INIReader& ini, std::string& host, std::string& address, uint16_t& port, size_t& buffer_size, uint16_t& max_thread)
{
    // host
    host = ini.GetString("http", "host", "127.0.0.1");
    assert(!host.empty() && "[http.host] Illegal host");

    // address
    address = ini.GetString("http", "address", "0.0.0.0");
    assert(!address.empty() && "[http.address] Illegal address");

    // port
    long _http_port = ini.GetInteger("http", "port", 8110);
    assert(std::in_range<uint16_t>(_http_port) && "[http.port] Must be within the range of [0-65535]");
    port = static_cast<uint16_t>(_http_port);

    // buffer size
    buffer_size = static_cast<size_t>(ini.GetUnsigned64("http", "buffer-size", 1024));
    assert((buffer_size >= 1024) && "[http.buffer-size] Must be >= 1024");

    // thread number
    long _max_thread = ini.GetInteger("http", "max_thread", 0);
    assert(std::in_range<uint16_t>(_max_thread) && "[http.max-thread] Must be within the range of [0-65535]");
    max_thread = static_cast<uint16_t>(_max_thread);
    if (max_thread == 0)
    {
        max_thread = std::thread::hardware_concurrency();
    }
}

static inline void ParseHttpsConfig(INIReader& ini, bool& enable, uint16_t& port, bool& only, std::filesystem::path& cert, std::filesystem::path& key)
{
    // https enable
    enable = ini.GetBoolean("https", "enable", "false");
    {
        long https_port = ini.GetInteger("https", "port", 8111);
        assert(std::in_range<uint16_t>(https_port) && "[https.port] Must be within the range of [0-65535]");
        port = static_cast<uint16_t>(https_port);
    }

    // https only
    only = ini.GetBoolean("https", "https-only", true);

    // https cert
    cert = ini.GetString("ssl", "cert", "");

    // https key
    key = ini.GetString("ssl", "key", "");

    if (enable)
    {
        assert(std::filesystem::is_regular_file(cert) && "[ssl.cert] Not exist");
        assert(std::filesystem::is_regular_file(key) && "[ssl.key] Not exist");
    }
}

static inline void ParseWebDAVConfig(INIReader& ini, std::string& prefix, std::string& route_prefix, std::filesystem::path& absolute_data_path,
                                     std::filesystem::path& relative_data_path, int8_t& max_recurse_depth, std::string& realm,
                                     std::string& verification, std::unordered_map<std::string, std::string>& users)
{
    // webdav prefix
    prefix = ini.GetString("webdav", "prefix", "/webdav");
    assert(!prefix.empty() && "[webdav.prefix] Cannot be empty");
    {
        char c = prefix.back();
        assert((c >= 65 && c <= 90) || (c >= 97 && c <= 122) && "[webdav.prefix] Must end with the [a-Z] character");
    }

    // webdav route prefix
    route_prefix = prefix + "/(.*)";

    // webdav data path
    const auto& cwd = std::filesystem::current_path();
    std::filesystem::path _data_path = ini.GetString("webdav", "data-path", "data");
    assert(std::filesystem::exists(_data_path) && "[webdav.data-path] Not exist");
    assert(std::filesystem::is_directory(_data_path) && "[webdav.data-path] Required directory");
    if (_data_path.is_absolute())
    {
        absolute_data_path = std::move(_data_path);
        relative_data_path = std::filesystem::relative(absolute_data_path, cwd).lexically_normal();
    }
    else
    {
        relative_data_path = std::move(_data_path);
        absolute_data_path = (cwd / relative_data_path).lexically_normal();
    }

    // webdav propfind max recurse depth
    long _max_recurse_depth = ini.GetInteger("webdav", "max-recurse-depth", 4);
    assert(std::in_range<int8_t>(_max_recurse_depth) && "[webdav.max-recurse-depth] Must be within the range of [0-255]");
    max_recurse_depth = static_cast<int8_t>(_max_recurse_depth);

    // webdav auth realm
    realm = ini.GetString("webdav", "realm", "WebDavRealm");
    assert(!realm.empty() && "[webdav.realm] Cannot be empty");

    // webdav verification type
    verification = ini.GetString("wevdav", "verification", "none");
    assert((verification == "basic" || verification == "digest" || verification == "none") &&
           "[webdav.verification] Must be one of them [basic|digest|none]");

    // webdav user list
    const std::string user_list_raw = ini.GetString("webdav", "users", "");
    for (const auto& user : utils::string::split(user_list_raw, ','))
    {
        users.emplace(utils::string::split2pair(user, '@'));
    }
}

static inline void ParseRedisConfig(INIReader& ini, std::string& host, uint16_t& port, std::string& user, std::string& passwd)
{
    // redis address
    host = ini.GetString("redis", "host", "localhost");

    // redis port
    long redis_port = ini.GetInteger("redis", "port", 6379);
    assert(std::in_range<uint16_t>(redis_port) && "redis port number out of range");
    port = static_cast<uint16_t>(redis_port);

    // redis user
    user = ini.GetString("redis", "username", "");
    assert(!user.empty() && "[redis.username] Cannot be empty");

    // redis password
    passwd = ini.GetString("redis", "password", "");
    assert(!passwd.empty() && "[redis.password] Cannot be empty");
}

static inline void ParseEngineConfig(INIReader& ini, std::string& etag_engine, std::string& prop_engine)
{
    std::unordered_set<std::string> engine_list{"memory", "sqlite", "redis"};

    etag_engine = ini.GetString("engine", "etag", "sqlite");
    assert(engine_list.contains(etag_engine) && "[engine.etag] Must be one of them [memory|sqlite|redis]");

    prop_engine = ini.GetString("engine", "prop", "sqlite");
    assert(engine_list.contains(prop_engine) && "[engine.prop] Must be one of them [memory|sqlite|redis]");
}

static inline void ParseCacheConfig(INIReader& ini, std::filesystem::path& sqlite_db, std::filesystem::path& etag_data,
                                    std::filesystem::path& prop_data)
{
    sqlite_db = ini.GetString("cache", "sqlite-db", "./data.db");
    etag_data = ini.GetString("cache", "etag-data", "./etag.dat");
    prop_data = ini.GetString("cache", "prop-data", "./prop.dat");
}

inline const void ConfigReader::WriteDefaultConfigFile(const std::filesystem::path& file)
{
    LOG_WARN("unable to read configuration file, creating ...")

    std::ofstream ofs(file, std::ios::out);
    if (!ofs.is_open())
    {
        LOG_ERROR("unable to write default configuration file, please check program permissions and try again.")
        return;
    }

    ofs << "[http]\n";
    ofs << "host = 127.0.0.1\n";
    ofs << "address = 0.0.0.0\n";
    ofs << "port = 8110\n";
    ofs << "buffer_size = 1024\n";
    ofs << "max_thread = 0\n";
    ofs << '\n';

    ofs << "[https]\n";
    ofs << "enable = false\n";
    ofs << "port = 8111\n";
    ofs << "https-only = true\n";
    ofs << "cert =./full_chain.pem\n";
    ofs << "key =./key.pem\n";
    ofs << '\n';

    ofs << "[webdav]\n";
    ofs << "data-path = ./data\n";
    ofs << "max-recurse-depth = 4\n";
    ofs << "realm = WebDavRealm\n";
    ofs << "verification = none\n";
    ofs << "users = test@admin\n";
    ofs << '\n';

    ofs << "[redis]\n";
    ofs << "host = localhost\n";
    ofs << "port = 6379\n";
    ofs << "username = default\n";
    ofs << "password =\n";
    ofs << '\n';

    ofs << "[engine]\n";
    ofs << "etag = sqlite\n";
    ofs << "prop = sqlite\n";
    ofs << '\n';

    ofs << "[cache]\n";
    ofs << "sqlite-db = ./data.db\n";
    ofs << "etag-data = ./etag.dat\n";
    ofs << "prop-data = ./prop.dat\n";

    ofs.flush();
    ofs.close();
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
        LOG_INFO("Default configuration has been generated, please restart the server.")
    }

    INIReader reader(path_string);
    assert(reader.ParseError() == 0 && "Failed to parsing settings.ini");
    cwd_ = fs::current_path();

    ParseHttpConfig(reader, http_host_, http_address_, http_port_, http_buffer_size_, http_max_thread_);

    ParseHttpsConfig(reader, https_enable_, https_port_, https_only_, ssl_cert_, ssl_key_);

    ParseWebDAVConfig(reader, webdav_prefix_, webdav_route_prefix_, webdav_absolute_data_path_, webdav_relative_data_path_, webdav_max_recurse_depth_,
                      webdav_realm_, webdav_verification_, webdav_user_);

    ParseRedisConfig(reader, redis_host_, redis_port_, redis_username_, redis_password_);

    ParseEngineConfig(reader, etag_engine_, prop_engine_);

    ParseCacheConfig(reader, sqlite_db_, etag_data_, prop_data_);
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

std::filesystem::path ConfigReader::GetWebDavRelativeDataPath(std::string_view url) const noexcept
{
    return std::filesystem::relative(GetWebDavAbsoluteDataPath(url), cwd_);
}

const std::filesystem::path& ConfigReader::GetWebDavAbsoluteDataPath() const noexcept
{
    return webdav_absolute_data_path_;
}

std::filesystem::path ConfigReader::GetWebDavAbsoluteDataPath(std::string_view url) const noexcept
{
    namespace fs = std::filesystem;

    if (url == webdav_prefix_)
    {
        return webdav_absolute_data_path_;
    }

    if (url.starts_with(webdav_prefix_))
    {
        url = url.substr(webdav_prefix_.size());
    }

    fs::path res = webdav_absolute_data_path_ / url;
    res = res.lexically_normal();
    // Escape from resource directory?
    {
        auto path = fs::relative(res, webdav_absolute_data_path_);
        auto path_str = utils::path::to_string(path);
        if (path_str.starts_with(".."))
        {
            LOG_WARN_FMT("Discovery of escape behavior from resource directory!\nTo: {}", path_str);
            return {""};
        }
    }

    return res;
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

std::string ConfigReader::GetWebDavUser(const std::string& user) const noexcept
{
    const auto& it = webdav_user_.find(user);
    if (it == webdav_user_.end())
    {
        return {""};
    }

    return it->second;
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

const std::filesystem::path& ConfigReader::GetPropData() const noexcept
{
    return prop_data_;
}
