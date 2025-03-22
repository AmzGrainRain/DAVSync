#include "ConfigManager.h"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>

#include "iguana/json_reader.hpp"
#include "iguana/json_writer.hpp"

const ConfigManager& ConfigManager::GetInstance(const std::filesystem::path& path, bool new_instance)
{
    static ConfigManager instance{path};

    if (new_instance) {
        instance.SaveConfig();
        instance = ConfigManager{path};
    }

    return instance;
}

inline void CheckHttpConfig(const HttpConfig& http_config)
{
    assert(!http_config.host.empty() && "[http.host] Illegal host");
    assert(!http_config.address.empty() && "[http.address] Illegal address");
    assert(std::in_range<uint16_t>(http_config.port) && "[http.port] Must be within the range of [0-65535]");
    assert((http_config.buffer_size >= 1024) && "[http.buffer_size] Must be >= 1024");
    assert(std::in_range<uint8_t>(http_config.max_thread) && "[http.max_thread] Must be within the range of [0-65535]");
}

inline void CheckHttpsConfig(const HttpsConfig& https_config)
{
    assert(std::in_range<uint16_t>(https_config.port) && "[https.port] Must be within the range of [0-65535]");
    if (https_config.enable)
    {
        assert(std::filesystem::is_regular_file(https_config.cert) && "[ssl.cert] Not exist");
        assert(std::filesystem::is_regular_file(https_config.key) && "[ssl.key] Not exist");
    }
}

inline void CheckWebDavConfig(const WebDavConfig& webdav_config, std::filesystem::path& abs_path, std::filesystem::path& rel_path,
                              std::string& route_prefix)
{

    assert(!webdav_config.prefix.empty() && "[webdav.prefix] Cannot be empty");
    {
        const char c = webdav_config.prefix.back();
        assert((c >= 65 && c <= 90) || (c >= 97 && c <= 122) && "[webdav.prefix] Must end with the [a-Z] character");
    }
    assert(std::filesystem::exists(webdav_config.data_path) && "[webdav.data_path] Not exist");
    assert(std::filesystem::is_directory(webdav_config.data_path) && "[webdav.data_path] Required directory");

    {
        if (std::filesystem::path data_path = webdav_config.data_path; data_path.is_absolute())
        {
            abs_path = std::move(data_path);
            rel_path = std::filesystem::relative(abs_path, std::filesystem::current_path()).lexically_normal();
        }
        else
        {
            rel_path = std::move(data_path);
            abs_path = (std::filesystem::current_path() / rel_path).lexically_normal();
        }
    }

    assert(std::in_range<int8_t>(webdav_config.max_recurse_depth) && "[webdav.max_recurse_depth] Must be within the range of [0-255]");
    assert(!webdav_config.realm.empty() && "[webdav.realm] Cannot be empty");
    assert((webdav_config.verification == "basic" || webdav_config.verification == "digest") &&
           "[webdav.verification] Must be one of them [basic|digest]");
    for (const auto& user : webdav_config.users)
    {
        assert(!user.name.empty() && "[webdav.users.name] Cannot be empty");
        assert(!user.password.empty() && "[webdav.users.password] Cannot be empty");
    }
    route_prefix = webdav_config.prefix + "/(.*)";
}

inline void CheckRedisConfig(const RedisConfig& redis_config)
{
    assert(!redis_config.host.empty() && "[redis.host] Cannot be empty");
    assert(std::in_range<uint16_t>(redis_config.port) && "[redis.port] Must be within the range of [0-65535]");
    assert(!redis_config.username.empty() && "[redis.username] Cannot be empty");
    assert(!redis_config.password.empty() && "[redis.password] Cannot be empty");
}

inline void CheckEngineConfig(const EngineConfig& engine_config)
{
    std::unordered_set<std::string> engine_list{"memory", "sqlite", "redis"};
    assert(engine_list.contains(engine_config.etag) && "[engine.etag] Must be one of them [memory|sqlite|redis]");
    assert(engine_list.contains(engine_config.prop) && "[engine.prop] Must be one of them [memory|sqlite|redis]");
}

ConfigManager::ConfigManager(const std::filesystem::path& config_file_path) : config_file_path_(config_file_path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(config_file_path))
    {
        CreateDefaultConfig();
        throw std::runtime_error("Default config file created, please modify it before running the program.");
    }

    LoadConfig(config_file_path);

    // Check HttpConfig
    CheckHttpConfig(config_.http);

    // Check HttpsConfig
    CheckHttpsConfig(config_.https);

    // Check WebDavConfig
    CheckWebDavConfig(config_.webdav, absolute_webdav_data_path_, relative_webdav_data_path_, webdav_prefix_);

    // Check RedisConfig
    CheckRedisConfig(config_.redis);

    // Check EngineConfig
    CheckEngineConfig(config_.engine);

    // The engine will create DataConfig, so it is not checked here.
}

void ConfigManager::SaveConfig() const
{
    std::ofstream file(config_file_path_, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + config_file_path_.string());
    }

    std::string buffer;
    iguana::to_json(config_, buffer);
    file << buffer;
    file.close();
}

void ConfigManager::LoadConfig(const std::filesystem::path& config_file_path)
{
    std::ifstream file(config_file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + config_file_path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    iguana::from_json(config_, buffer.str());
}

void ConfigManager::ReloadConfig()
{
    config_ = Config{};
    LoadConfig(config_file_path_);
}

const Config& ConfigManager::GetGlobalConfig() const noexcept
{
    return config_;
}

const HttpConfig& ConfigManager::GetHttpConfig() const noexcept
{
    return config_.http;
}

const HttpsConfig& ConfigManager::GetHttpsConfig() const noexcept
{
    return config_.https;
}

const WebDavConfig& ConfigManager::GetWebDavConfig() const noexcept
{
    return config_.webdav;
}

const RedisConfig& ConfigManager::GetRedisConfig() const noexcept
{
    return config_.redis;
}

const EngineConfig& ConfigManager::GetEngineConfig() const noexcept
{
    return config_.engine;
}

const DataConfig& ConfigManager::GetDataConfig() const noexcept
{
    return config_.data;
}

void ConfigManager::CreateDefaultConfig() const
{
    if (std::filesystem::exists(config_file_path_))
    {
        return;
    }

    Config config;
    config.http.host = "127.0.0.1";
    config.http.address = "0.0.0.0";
    config.http.port = 8110;
    config.http.buffer_size = 1024;
    config.http.max_thread = 0;

    config.https.enable = false;
    config.https.port = 8111;
    config.https.https_only = false;
    config.https.cert = "fullchain.pem";
    config.https.key = "privkey.pem";

    config.webdav.prefix = "/webdav";
    config.webdav.data_path = "./data";
    config.webdav.max_recurse_depth = 4;
    config.webdav.realm = "WEBDAV_REALM";
    config.webdav.verification = "basic";
    config.webdav.users.emplace_back("test", "passw0rd");

    config.redis.host = "127.0.0.1";
    config.redis.port = 6379;
    config.redis.username = "test";
    config.redis.password = "test";

    config.engine.etag = "sqlite";
    config.engine.prop = "sqlite";

    config.data.sqlite_db = "./metadata/data.db";
    config.data.etag_data = "./metadata/etag.db";
    config.data.prop_data = "./metadata/prop.db";

    std::ofstream file(config_file_path_, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + config_file_path_.string());
    }

    std::string buffer;
    iguana::to_json(config, buffer);
    file << buffer;
    file.close();
}

const std::string& ConfigManager::GetHttpHost() const noexcept
{
    return config_.http.host;
}

const std::string& ConfigManager::GetHttpAddress() const noexcept
{
    return config_.http.address;
}

uint16_t ConfigManager::GetHttpPort() const noexcept
{
    return static_cast<uint16_t>(config_.http.port);
}

uint16_t ConfigManager::GetHttpMaxThread() const noexcept
{
    return static_cast<uint16_t>(config_.http.max_thread);
}

size_t ConfigManager::GetHttpBufferSize() const noexcept
{
    return config_.http.buffer_size;
}

bool ConfigManager::GetHttpsEnabled() const noexcept
{
    return config_.https.enable;
}

uint16_t ConfigManager::GetHttpsPort() const noexcept
{
    return static_cast<uint16_t>(config_.https.port);
}

bool ConfigManager::GetHttpsOnly() const noexcept
{
    return config_.https.https_only;
}

const std::filesystem::path& ConfigManager::GetSSLCertPath() const noexcept
{
    static std::filesystem::path cert_path = config_.https.cert;
    return cert_path;
}

const std::filesystem::path& ConfigManager::GetSSLKeyPath() const noexcept
{
    static std::filesystem::path key_path = config_.https.key;
    return key_path;
}

const std::string& ConfigManager::GetWebDavPrefix() const noexcept
{
    return config_.webdav.prefix;
}

const std::string& ConfigManager::GetWebDavRoutePrefix() const noexcept
{
    return route_prefix_;
}

const std::filesystem::path& ConfigManager::GetWebDavRelativeDataPath() const noexcept
{
    return relative_webdav_data_path_;
}

std::filesystem::path ConfigManager::GetWebDavRelativeDataPath(std::string_view url) const noexcept
{
    return relative(GetWebDavAbsoluteDataPath(url), std::filesystem::current_path());
}

const std::filesystem::path& ConfigManager::GetWebDavAbsoluteDataPath() const noexcept
{
    return absolute_webdav_data_path_;
}

std::filesystem::path ConfigManager::GetWebDavAbsoluteDataPath(std::string_view url) const noexcept
{
    namespace fs = std::filesystem;

    if (url == webdav_prefix_)
    {
        return absolute_webdav_data_path_;
    }

    if (url.starts_with(webdav_prefix_))
    {
        url = url.substr(webdav_prefix_.size());
    }

    fs::path res = absolute_webdav_data_path_ / url;
    res = res.lexically_normal();
    // Escape from resource directory?
    {
        fs::path path = fs::relative(res, absolute_webdav_data_path_);
        std::string path_str = path.string();
        if (path_str.starts_with(".."))
        {
            // LOG_WARN_FMT("Discovery of escape behavior from resource directory!\nTo: {}", path_str);
            return {""};
        }
    }

    return res;
}

int8_t ConfigManager::GetWebDavMaxRecurseDepth() const noexcept
{
    return static_cast<int8_t>(config_.webdav.max_recurse_depth);
}

const std::string& ConfigManager::GetWebDavRealm() const noexcept
{
    return config_.webdav.realm;
}

const std::string& ConfigManager::GetWebDavVerification() const noexcept
{
    return config_.webdav.verification;
}

auto ConfigManager::GetWebDavUser(const std::string& user) const noexcept -> std::optional<WebDavUser>
{
    for (const auto& u : config_.webdav.users)
    {
        if (u.name == user)
        {
            return u;
        }
    }
    return std::nullopt;
}

const std::string& ConfigManager::GetRedisHost() const noexcept
{
    return config_.redis.host;
}

uint16_t ConfigManager::GetRedisPort() const noexcept
{
    return static_cast<uint16_t>(config_.redis.port);
}

const std::string& ConfigManager::GetRedisUserName() const noexcept
{
    return config_.redis.username;
}

const std::string& ConfigManager::GetRedisPassword() const noexcept
{
    return config_.redis.password;
}

const std::string& ConfigManager::GetETagEngine() const noexcept
{
    return config_.engine.etag;
}

const std::string& ConfigManager::GetPropEngine() const noexcept
{
    return config_.engine.prop;
}

const std::filesystem::path& ConfigManager::GetSQLiteDB() const noexcept
{
    static std::filesystem::path path = config_.data.sqlite_db;
    return path;
}

const std::filesystem::path& ConfigManager::GetETagData() const noexcept
{
    static std::filesystem::path path = config_.data.etag_data;
    return path;
}

const std::filesystem::path& ConfigManager::GetPropData() const noexcept
{
    static std::filesystem::path path = config_.data.prop_data;
    return path;
}
