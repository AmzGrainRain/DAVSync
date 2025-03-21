#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

class ConfigReader
{
  public:
    explicit ConfigReader(const std::filesystem::path& path = std::filesystem::current_path() / "settings.ini");

    static const ConfigReader& GetInstance();

    static void WriteDefaultConfigFile(const std::filesystem::path& file);

    [[nodiscard]] const std::filesystem::path& GetCWD() const noexcept;

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
    [[nodiscard]] std::string GetWebDavUser(const std::string& user) const noexcept;

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
    std::filesystem::path cwd_;

    std::string http_host_;
    std::string http_address_;
    uint16_t http_port_;
    uint16_t http_max_thread_;
    size_t http_buffer_size_;

    bool https_enable_;
    uint16_t https_port_;
    bool https_only_;
    std::filesystem::path ssl_cert_;
    std::filesystem::path ssl_key_;

    std::string webdav_prefix_;
    std::string webdav_route_prefix_;
    std::filesystem::path webdav_relative_data_path_;
    std::filesystem::path webdav_absolute_data_path_;
    int8_t webdav_max_recurse_depth_;
    std::string webdav_realm_;
    std::string webdav_verification_;
    std::unordered_map<std::string, std::string> webdav_user_;

    std::string redis_host_;
    uint16_t redis_port_;
    std::string redis_username_;
    std::string redis_password_;

    std::string etag_engine_;
    std::string prop_engine_;

    std::filesystem::path sqlite_db_;
    std::filesystem::path etag_data_;
    std::filesystem::path prop_data_;
};
