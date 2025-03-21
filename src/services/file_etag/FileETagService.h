#pragma once

#include <filesystem>
#include <string>

namespace FileETagService
{

struct FileETagTable
{
  std::string path;
  std::string sha;
};

class FileETagService
{
  public:
    virtual ~FileETagService() = default;
    virtual std::string Get(const std::filesystem::path& path) noexcept = 0;
    virtual std::string Set(const std::filesystem::path& path) noexcept = 0;
};

} // namespace FileETagService
