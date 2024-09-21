#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "FileETagService.h"

namespace FileETagService
{

class MemoryFileETagService : public FileETagService
{
  public:
    using ETagContainer = std::unordered_map<std::string, std::string>;

    MemoryFileETagService();
    ~MemoryFileETagService();

    std::string Get(const std::string& path_sha) override;

    bool Set(const std::filesystem::path& file) override;

  private:
    ETagContainer etag_map_;
    std::ofstream data_;
};

} // namespace FileETagService
