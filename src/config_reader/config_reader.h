#pragma once

#include <filesystem>
#include <string>
#include <cstdint>

class ConfigReader
{
  public:
    ConfigReader(const std::filesystem::path& path = "./settings.ini");

    const std::string& GetHttpHost() const noexcept;

    uint16_t GetHttpPort() const noexcept;

    uint8_t GetHttpMaxThread() const noexcept;

    const std::string& GetRedisHost() const noexcept;

    uint16_t GetRedisPort() const noexcept;

    const std::string& GetRedisAuth() const noexcept;

    const std::string& GetUserName() const noexcept;

    const std::string& GetUserPassword() const noexcept;

  private:
    std::string http_host_;
    uint16_t http_port_;
    uint8_t http_max_thread_;

    std::string redis_host_;
    uint16_t redis_port_;
    std::string redis_auth_;

    std::string user_name_;
    std::string user_passwd_;
};
