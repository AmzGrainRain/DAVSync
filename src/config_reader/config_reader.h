#pragma once

#include <filesystem>
#include <string>
#include <cstdint>

class ConfigReader
{
  public:
    ConfigReader(const std::filesystem::path& path = "./settings.ini");

    static const ConfigReader& GetInstance();

    const std::string& GetHttpHost() const noexcept;
    uint16_t GetHttpPort() const noexcept;
    uint8_t GetHttpMaxThread() const noexcept;

    const std::string& GetWebDavPrefix() const noexcept;
    const std::string& GetWebDavVerification() const noexcept;
    const std::string& GetWebDavRealm() const noexcept;

    const std::string& GetRedisHost() const noexcept;
    uint16_t GetRedisPort() const noexcept;
    const std::string& GetRedisAuth() const noexcept;

    const std::string& GetUserAccount() const noexcept;
    const std::string& GetUserPassword() const noexcept;

  private:
    std::string http_host_;
    short http_port_;
    uint8_t http_max_thread_;

    std::string webdav_prefix_;
    std::string webdav_verification_;
    std::string webdav_realm_;

    std::string redis_host_;
    short redis_port_;
    std::string redis_auth_;

    std::string user_account_;
    std::string user_passwd_;
};
