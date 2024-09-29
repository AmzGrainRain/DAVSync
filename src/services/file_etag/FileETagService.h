#pragma once

#include <filesystem>
#include <string>

#include <PicoSHA2/picosha2.h>

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
    virtual std::string Get(const std::filesystem::path& path) noexcept = 0;
    virtual std::string Set(const std::filesystem::path& path) noexcept = 0;
};

} // namespace FileETagService
