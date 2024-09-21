#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FilePropService
{

struct FilePropTable
{
    std::string sha;
    std::string key;
    std::string value;
    size_t id;
};

using PropT = std::pair<std::string, std::string>;

class FilePropService
{
  public:
    virtual bool Set(const std::string& path_sha, const PropT& prop) = 0;

    virtual std::string Get(const std::string& path_sha, const std::string& key) = 0;

    virtual std::vector<PropT> GetAll(const std::string& path_sha) = 0;

    virtual bool Remove(const std::string& path_sha, const std::string& key) = 0;

    virtual bool RemoveAll(const std::string& path_sha) = 0;
};

} // namespace FilePropService
