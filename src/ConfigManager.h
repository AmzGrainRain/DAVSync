#pragma once

#include <filesystem>
#include <string>

struct HttpConfig
{
    std::string host;
    std::string address;
    int port{};
    size_t buffer_size{};
    int max_thread{};
};

struct HttpsConfig
{
    bool enable{};
    int port{};
    bool https_only{};
    std::string cert;
    std::string key;
};

struct WebDavUser
{
    std::string name;
    std::string password;
};

struct WebDavConfig
{
    std::string prefix;
    std::string data_path;
    int max_recurse_depth{};
    std::string realm;
    std::string verification;
    std::vector<WebDavUser> users;
};

struct RedisConfig
{
    std::string host;
    int port{};
    std::string username;
    std::string password;
};

struct EngineConfig
{
    std::string etag;
    std::string prop;
};

struct DataConfig
{
    std::string sqlite_db;
    std::string etag_data;
    std::string prop_data;
};

struct Config
{
    HttpConfig http;
    HttpsConfig https;
    WebDavConfig webdav;
    RedisConfig redis;
    EngineConfig engine;
    DataConfig data;
};

class ConfigManager
{
public:

    explicit ConfigManager(const std::filesystem::path& config_file_path = std::filesystem::current_path() / "settings.json");
    static const ConfigManager& GetInstance(const std::filesystem::path& path = std::filesystem::current_path() / "settings.json", bool new_instance = false);

    void SaveConfig() const;
    void LoadConfig(const std::filesystem::path& path);
    void ReloadConfig();

    [[nodiscard]] const Config& GetGlobalConfig() const noexcept;
    [[nodiscard]] const HttpConfig& GetHttpConfig() const noexcept;
    [[nodiscard]] const HttpsConfig& GetHttpsConfig() const noexcept;
    [[nodiscard]] const WebDavConfig& GetWebDavConfig() const noexcept;
    [[nodiscard]] const RedisConfig& GetRedisConfig() const noexcept;
    [[nodiscard]] const EngineConfig& GetEngineConfig() const noexcept;
    [[nodiscard]] const DataConfig& GetDataConfig() const noexcept;

    [[nodiscard]] const std::string& GetHttpHost() const noexcept;
    [[nodiscard]] const std::string& GetHttpAddress() const noexcept;
    [[nodiscard]] uint16_t GetHttpPort() const noexcept;
    [[nodiscard]] uint16_t GetHttpMaxThread() const noexcept;
    [[nodiscard]] size_t GetHttpBufferSize() const noexcept;
    [[nodiscard]] bool GetHttpsEnabled() const noexcept;
    [[nodiscard]] uint16_t GetHttpsPort() const noexcept;
    [[nodiscard]] bool GetHttpsOnly() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetSSLCertPath() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetSSLKeyPath() const noexcept;
    [[nodiscard]] const std::string& GetWebDavPrefix() const noexcept;
    [[nodiscard]] const std::string& GetWebDavRoutePrefix() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetWebDavRelativeDataPath() const noexcept;
    [[nodiscard]] std::filesystem::path GetWebDavRelativeDataPath(std::string_view url) const noexcept;
    [[nodiscard]] const std::filesystem::path& GetWebDavAbsoluteDataPath() const noexcept;
    [[nodiscard]] std::filesystem::path GetWebDavAbsoluteDataPath(std::string_view url) const noexcept;
    [[nodiscard]] int8_t GetWebDavMaxRecurseDepth() const noexcept;
    [[nodiscard]] const std::string& GetWebDavRealm() const noexcept;
    [[nodiscard]] const std::string& GetWebDavVerification() const noexcept;
    [[nodiscard]] auto GetWebDavUser(const std::string& user) const noexcept -> std::optional<WebDavUser>;
    [[nodiscard]] const std::string& GetRedisHost() const noexcept;
    [[nodiscard]] uint16_t GetRedisPort() const noexcept;
    [[nodiscard]] const std::string& GetRedisUserName() const noexcept;
    [[nodiscard]] const std::string& GetRedisPassword() const noexcept;
    [[nodiscard]] const std::string& GetETagEngine() const noexcept;
    [[nodiscard]] const std::string& GetPropEngine() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetSQLiteDB() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetETagData() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetPropData() const noexcept;

private:
    void CreateDefaultConfig() const;

    static ConfigManager* instance_;

    Config config_;
    std::filesystem::path config_file_path_;

    std::string webdav_prefix_;
    std::string route_prefix_;
    std::filesystem::path absolute_webdav_data_path_;
    std::filesystem::path relative_webdav_data_path_;
};
