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
    using ETagMapKeyT = std::filesystem::path;
    using ETagMapValueT = std::string;
    using ETagMapT = std::unordered_map<ETagMapKeyT, ETagMapValueT>;

    MemoryFileETagService();

    ~MemoryFileETagService();

    std::string Get(const std::filesystem::path& path) noexcept override;

    std::string Set(const std::filesystem::path& path) noexcept override;

  private:
    ETagMapT etag_map_;
    std::ofstream data_;
};

} // namespace FileETagService
