#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "FilePropService.h"

namespace FilePropService
{

class MemoryFilePropService : public FilePropService
{
  public:
    using ETagMapKeyT = std::filesystem::path;
    using ETagMapValueT = std::unordered_map<std::string, std::string>;
    using ETagMapT = std::unordered_map<ETagMapKeyT, ETagMapValueT>;

    MemoryFilePropService();
    ~MemoryFilePropService();

    bool Set(const std::filesystem::path& path, const PropT& prop) override;

    std::string Get(const std::filesystem::path& path, const std::string& key) override;

    std::vector<PropT> GetAll(const std::filesystem::path& path) override;

    bool Remove(const std::filesystem::path& path, const std::string& key) override;

    bool RemoveAll(const std::filesystem::path& path) override;

  private:
    ETagMapT prop_map_;
    std::ofstream data_;
};

} // namespace FilePropService
