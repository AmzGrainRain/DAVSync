#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace FilePropService
{

struct FilePropTable
{
    std::string path;
    std::string key;
    std::string value;
    size_t id;
};

using PropT = std::pair<std::string, std::string>;

class FilePropService
{
  public:
    virtual bool Set(const std::filesystem::path& path, const PropT& prop) noexcept = 0;

    virtual std::string Get(const std::filesystem::path& path, const std::string& key) noexcept = 0;

    virtual std::vector<PropT> GetAll(const std::filesystem::path& path) noexcept = 0;

    virtual bool Remove(const std::filesystem::path& path, const std::string& key) noexcept = 0;

    virtual bool RemoveAll(const std::filesystem::path& path) noexcept = 0;
};

} // namespace FilePropService
