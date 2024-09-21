#pragma once

#include <fstream>
#include <string>
#include <unordered_map>

#include "FilePropService.h"

namespace FilePropService
{

class MemoryFilePropService : public FilePropService
{
  public:
    using ETagContainerItem = std::unordered_map<std::string, std::string>;
    using ETagContainer = std::unordered_map<std::string, ETagContainerItem>;

    MemoryFilePropService();
    ~MemoryFilePropService();

    bool Set(const std::string& path_sha, const PropT& prop) override;

    std::string Get(const std::string& path_sha, const std::string& key) override;

    std::vector<PropT> GetAll(const std::string& path_sha) override;

    bool Remove(const std::string& path_sha, const std::string& key) override;

    bool RemoveAll(const std::string& path_sha) override;

  private:
    ETagContainer prop_map_;
    std::ofstream data_;
};

} // namespace FilePropService
