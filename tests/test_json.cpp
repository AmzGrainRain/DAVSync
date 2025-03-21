#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <iguana/json_reader.hpp>
#include <iguana/json_writer.hpp>

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

struct Settings
{
    HttpConfig http;
    HttpsConfig https;
    WebDavConfig webdav;
    RedisConfig redis;
    EngineConfig engine;
    DataConfig data;
};

class SettingsManager
{
  public:
    friend void test();

    static SettingsManager& GetInstance(const std::filesystem::path& path)
    {
        static SettingsManager instance(path);
        return instance;
    }

    void SaveConfig()
    {
        std::ofstream file(settings_file_path_, std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            std::cerr << "Failed to open " << settings_file_path_ << std::endl;
            throw std::runtime_error("Failed to open " + settings_file_path_.string());
        }

        std::string buffer;
        iguana::to_json(settings_, buffer);
        file << buffer;
        file.close();
    }

    void ReloadConfig()
    {
        settings_ = Settings();
        LoadConfig(settings_file_path_);
    }

    const std::string& GetHttpHost() const noexcept
    {
        return settings_.http.host;
    }

    const std::string& GetHttpAddress() const noexcept
    {
        return settings_.http.address;
    }

    uint16_t GetHttpPort() const noexcept
    {
        return static_cast<uint16_t>(settings_.http.port);
    }

    uint16_t GetHttpMaxThread() const noexcept
    {
        return static_cast<uint16_t>(settings_.http.max_thread);
    }

    size_t GetHttpBufferSize() const noexcept
    {
        return settings_.http.buffer_size;
    }

    bool GetHttpsEnabled() const noexcept
    {
        return settings_.https.enable;
    }

    uint16_t GetHttpsPort() const noexcept
    {
        return static_cast<uint16_t>(settings_.https.port);
    }

    bool GetHttpsOnly() const noexcept
    {
        return settings_.https.https_only;
    }

    const std::filesystem::path& GetSSLCertPath() const noexcept
    {
        static std::filesystem::path cert_path = settings_.https.cert;
        return cert_path;
    }

    const std::filesystem::path& GetSSLKeyPath() const noexcept
    {
        static std::filesystem::path key_path = settings_.https.key;
        return key_path;
    }

    const std::string& GetWebDavPrefix() const noexcept
    {
        return settings_.webdav.prefix;
    }

    const std::string& GetWebDavRoutePrefix() const noexcept
    {
        return route_prefix_;
    }

    const std::filesystem::path& GetWebDavRelativeDataPath() const noexcept
    {
        return relative_webdav_data_path_;
    }

    std::filesystem::path GetWebDavRelativeDataPath(std::string_view url) const noexcept
    {
        return relative(GetWebDavAbsoluteDataPath(url), std::filesystem::current_path());
    }

    const std::filesystem::path& GetWebDavAbsoluteDataPath() const noexcept
    {
        return absolute_webdav_data_path_;
    }

    std::filesystem::path GetWebDavAbsoluteDataPath(std::string_view url) const noexcept
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

    int8_t GetWebDavMaxRecurseDepth() const noexcept
    {
        return static_cast<int8_t>(settings_.webdav.max_recurse_depth);
    }

    const std::string& GetWebDavRealm() const noexcept
    {
        return settings_.webdav.realm;
    }

    const std::string& GetWebDavVerification() const noexcept
    {
        return settings_.webdav.verification;
    }

    auto GetWebDavUser(const std::string& user) const noexcept -> std::optional<WebDavUser>
    {
        for (const auto& u : settings_.webdav.users)
        {
            if (u.name == user)
            {
                return u;
            }
        }
        return std::nullopt;
    }


    const std::string& GetRedisHost() const noexcept
    {
        return settings_.redis.host;
    }

    uint16_t GetRedisPort() const noexcept
    {
        return static_cast<uint16_t>(settings_.redis.port);
    }

    const std::string& GetRedisUserName() const noexcept
    {
        return settings_.redis.username;
    }

    const std::string& GetRedisPassword() const noexcept
    {
        return settings_.redis.password;
    }

    const std::string& GetETagEngine() const noexcept
    {
        return settings_.engine.etag;
    }

    const std::string& GetPropEngine() const noexcept
    {
        return settings_.engine.prop;
    }

    const std::filesystem::path& GetSQLiteDB() const noexcept
    {
        static std::filesystem::path path = settings_.data.sqlite_db;
        return path;
    }

    const std::filesystem::path& GetETagData() const noexcept
    {
        static std::filesystem::path path = settings_.data.etag_data;
        return path;
    }

    const std::filesystem::path& GetPropData() const noexcept
    {
        static std::filesystem::path path = settings_.data.prop_data;
        return path;
    }

  private:
    explicit SettingsManager(const std::filesystem::path& file_path) : settings_file_path_(file_path)
    {
        if (!std::filesystem::exists(file_path))
        {
            CreateDefaultConfig(file_path);
            throw std::runtime_error("Default config file created, please modify it before running the program.");
        }
        LoadConfig(settings_file_path_);

        // http
        assert(!settings_.http.host.empty() && "[http.host] Illegal host");
        assert(!settings_.http.address.empty() && "[http.address] Illegal address");
        assert(std::in_range<uint16_t>(settings_.http.port) && "[http.port] Must be within the range of [0-65535]");
        assert((settings_.http.buffer_size >= 1024) && "[http.buffer_size] Must be >= 1024");
        assert(std::in_range<uint8_t>(settings_.http.max_thread) && "[http.max_thread] Must be within the range of [0-65535]");

        // https
        assert(std::in_range<uint16_t>(settings_.https.port) && "[https.port] Must be within the range of [0-65535]");
        if (settings_.https.enable)
        {
            assert(std::filesystem::is_regular_file(settings_.https.cert) && "[ssl.cert] Not exist");
            assert(std::filesystem::is_regular_file(settings_.https.key) && "[ssl.key] Not exist");
        }

        // webdav
        assert(!settings_.webdav.prefix.empty() && "[webdav.prefix] Cannot be empty");
        {
            const char c = settings_.webdav.prefix.back();
            assert((c >= 65 && c <= 90) || (c >= 97 && c <= 122) && "[webdav.prefix] Must end with the [a-Z] character");
        }
        assert(std::filesystem::exists(settings_.webdav.data_path) && "[webdav.data_path] Not exist");
        assert(std::filesystem::is_directory(settings_.webdav.data_path) && "[webdav.data_path] Required directory");

        {
            if (std::filesystem::path data_path = settings_.webdav.data_path; data_path.is_absolute())
            {
                absolute_webdav_data_path_ = std::move(data_path);
                relative_webdav_data_path_ =
                    std::filesystem::relative(absolute_webdav_data_path_, std::filesystem::current_path()).lexically_normal();
            }
            else
            {
                relative_webdav_data_path_ = std::move(data_path);
                absolute_webdav_data_path_ = (std::filesystem::current_path() / relative_webdav_data_path_).lexically_normal();
            }
        }

        assert(std::in_range<int8_t>(settings_.webdav.max_recurse_depth) && "[webdav.max_recurse_depth] Must be within the range of [0-255]");
        assert(!settings_.webdav.realm.empty() && "[webdav.realm] Cannot be empty");
        assert((settings_.webdav.verification == "basic" || settings_.webdav.verification == "digest") &&
               "[webdav.verification] Must be one of them [basic|digest]");
        for (const auto& user : settings_.webdav.users)
        {
            assert(!user.name.empty() && "[webdav.users.name] Cannot be empty");
            assert(!user.password.empty() && "[webdav.users.password] Cannot be empty");
        }
        route_prefix_ = settings_.webdav.prefix + "/(.*)";

        // redis
        assert(!settings_.redis.host.empty() && "[redis.host] Cannot be empty");
        assert(std::in_range<uint16_t>(settings_.redis.port) && "[redis.port] Must be within the range of [0-65535]");
        assert(!settings_.redis.username.empty() && "[redis.username] Cannot be empty");
        assert(!settings_.redis.password.empty() && "[redis.password] Cannot be empty");

        // engine
        std::unordered_set<std::string> engine_list{"memory", "sqlite", "redis"};
        assert(engine_list.contains(settings_.engine.etag) && "[engine.etag] Must be one of them [memory|sqlite|redis]");
        assert(engine_list.contains(settings_.engine.prop) && "[engine.prop] Must be one of them [memory|sqlite|redis]");
    }

    void CreateDefaultConfig(const std::filesystem::path& file_path)
    {
        if (std::filesystem::exists(file_path))
            return;

        Settings settings;
        settings.http.host = "127.0.0.1";
        settings.http.address = "0.0.0.0";
        settings.http.port = 8110;
        settings.http.buffer_size = 1024;
        settings.http.max_thread = 0;

        settings.https.enable = false;
        settings.https.port = 8111;
        settings.https.https_only = false;
        settings.https.cert = "fullchain.pem";
        settings.https.key = "privkey.pem";

        settings.webdav.prefix = "/webdav";
        settings.webdav.data_path = "./data";
        settings.webdav.max_recurse_depth = 4;
        settings.webdav.realm = "WEBDAV_REALM";
        settings.webdav.verification = "basic";
        settings.webdav.users.emplace_back("test", "passw0rd");

        settings.redis.host = "127.0.0.1";
        settings.redis.port = 6379;
        settings.redis.username = "";
        settings.redis.password = "";

        settings.engine.etag = "sqlite";
        settings.engine.prop = "sqlite";

        settings.data.sqlite_db = "./metadata/data.db";
        settings.data.etag_data = "./metadata/etag.db";
        settings.data.prop_data = "./metadata/prop.db";

        std::ofstream settings_file(file_path, std::ios::out | std::ios::trunc);
        if (!settings_file.is_open())
        {
            std::cerr << "Failed to open " << file_path << std::endl;
            throw std::runtime_error("Failed to open " + file_path.string());
        }

        std::string buffer;
        iguana::to_json(settings, buffer);
        settings_file << buffer;
        settings_file.close();
    }

    void LoadConfig(const std::filesystem::path& file_path)
    {
        std::ifstream file(settings_file_path_);
        if (!file.is_open())
        {
            std::cerr << "Failed to open " << settings_file_path_ << std::endl;
            throw std::runtime_error("Failed to open " + settings_file_path_.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        iguana::from_json(settings_, buffer.str());
    }

    Settings settings_;
    std::filesystem::path settings_file_path_;

    std::string webdav_prefix_;
    std::string route_prefix_;
    std::filesystem::path absolute_webdav_data_path_;
    std::filesystem::path relative_webdav_data_path_;
};

void test()
{
    auto config = SettingsManager::GetInstance("../../settings.json");

    std::cout << "HTTP Host: " << config.settings_.http.host << std::endl;
    std::cout << "HTTP Address: " << config.settings_.http.address << std::endl;
    std::cout << "HTTP Port: " << config.settings_.http.port << std::endl;
    std::cout << "HTTP Buffer Size: " << config.settings_.http.buffer_size << std::endl;
    std::cout << "HTTP Max Thread: " << config.settings_.http.max_thread << std::endl;

    std::cout << "HTTPS Enable: " << config.settings_.https.enable << std::endl;
    std::cout << "HTTPS Port: " << config.settings_.https.port << std::endl;
    std::cout << "HTTPS HTTPS Only: " << config.settings_.https.https_only << std::endl;
    std::cout << "HTTPS Cert: " << config.settings_.https.cert << std::endl;
    std::cout << "HTTPS Key: " << config.settings_.https.key << std::endl;

    std::cout << "WebDav Prefix: " << config.settings_.webdav.prefix << std::endl;
    std::cout << "WebDav Data Path: " << config.settings_.webdav.data_path << std::endl;
    std::cout << "WebDav Max Recurse Depth: " << config.settings_.webdav.max_recurse_depth << std::endl;
    std::cout << "WebDav Realm: " << config.settings_.webdav.realm << std::endl;
    std::cout << "WebDav Verification: " << config.settings_.webdav.verification << std::endl;
    for (const auto& [name, password] : config.settings_.webdav.users)
    {
        std::cout << "WebDav User Name: " << name << std::endl;
        std::cout << "WebDav User Password: " << password << std::endl;
    }

    std::cout << "Redis Host: " << config.settings_.redis.host << std::endl;
    std::cout << "Redis Port: " << config.settings_.redis.port << std::endl;
    std::cout << "Redis Username: " << config.settings_.redis.username << std::endl;
    std::cout << "Redis Password: " << config.settings_.redis.password << std::endl;

    std::cout << "Engine ETag: " << config.settings_.engine.etag << std::endl;
    std::cout << "Engine Prop: " << config.settings_.engine.prop << std::endl;

    std::cout << "Data SQLite DB: " << config.settings_.data.sqlite_db << std::endl;
    std::cout << "Data ETag Data: " << config.settings_.data.etag_data << std::endl;
    std::cout << "Data Prop Data: " << config.settings_.data.prop_data << std::endl;
}

int main() {
    test();
    return 0;
}
