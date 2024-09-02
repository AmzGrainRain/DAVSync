#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>

class ConfigReader
{
  public:
    ConfigReader(const std::filesystem::path& path = std::filesystem::current_path() / "settings.ini");

    static const ConfigReader& GetInstance();

    inline const void WriteDefaultConfigFile(const std::filesystem::path& file);

    const std::filesystem::path& GetCWD() const noexcept;

    const std::string& GetHttpHost() const noexcept;
    const std::string& GetHttpAddress() const noexcept;
    uint16_t GetHttpPort() const noexcept;
    uint16_t GetHttpMaxThread() const noexcept;
    size_t GetHttpBufferSize() const noexcept;

    bool GetSSLEnabled() const noexcept;
    const std::filesystem::path& GetSSLCertPath() const noexcept;
    const std::filesystem::path& GetSSLKeyPath() const noexcept;

    const std::string& GetWebDavPrefix() const noexcept;
    const std::string& GetWebDavRoutePrefix() const noexcept;
    const std::filesystem::path& GetWebDavRelativeDataPath() const noexcept;
    const std::filesystem::path& GetWebDavAbsoluteDataPath() const noexcept;
    const std::string& GetWebDavVerification() const noexcept;
    const std::string& GetWebDavRealm() const noexcept;
    int8_t GetWebDavMaxRecurseDepth() const noexcept;

    bool GetSQLiteEnable() const noexcept;
    const std::filesystem::path& GetSQLiteLocation() const noexcept;

    bool GetRedisEnable() const noexcept;
    const std::string& GetRedisHost() const noexcept;
    uint16_t GetRedisPort() const noexcept;
    const std::string& GetRedisAuth() const noexcept;

    const std::string& GetUserAccount() const noexcept;
    const std::string& GetUserPassword() const noexcept;

  private:
    std::filesystem::path cwd_;

    std::string http_host_;
    std::string http_address_;
    uint16_t http_port_;
    uint16_t http_max_thread_;
    size_t http_buffer_size_;

    bool ssl_enable_;
    std::filesystem::path ssl_cert_path_;
    std::filesystem::path ssl_key_path_;

    std::string webdav_prefix_;
    std::string webdav_route_prefix_;
    std::filesystem::path webdav_relative_data_path_;
    std::filesystem::path webdav_absolute_data_path_;
    std::string webdav_verification_;
    std::string webdav_realm_;
    int8_t webdav_max_recurse_depth_;

    bool sqlite_enable_;
    std::filesystem::path sqlite_location_;

    bool pgsql_enable_;
    std::string pgsql_host_;
    uint16_t pgsql_port_;
    std::string pgsql_database_;
    std::string pgsql_user_;
    std::string pgsql_password_;

    bool redis_enable_;
    std::string redis_host_;
    uint16_t redis_port_;
    std::string redis_auth_;

    std::string user_account_;
    std::string user_passwd_;
};
