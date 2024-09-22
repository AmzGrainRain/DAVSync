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
  size_t id;
};

class FileETagService
{
  public:
    virtual std::string Get(const std::filesystem::path& path) = 0;
    virtual bool Set(const std::filesystem::path& path) = 0;
};

} // namespace FileETagService
