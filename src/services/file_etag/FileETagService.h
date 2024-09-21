#pragma once

#include <filesystem>
#include <string>

#include <PicoSHA2/picosha2.h>

namespace FileETagService
{

struct FileETagTable
{
  std::string path;
  std::string path_sha;
  std::string sha;
  size_t id;
};

class FileETagService
{
  public:
    virtual std::string Get(const std::string& path_sha) = 0;
    virtual bool Set(const std::filesystem::path& file) = 0;
};

} // namespace FileETagService
