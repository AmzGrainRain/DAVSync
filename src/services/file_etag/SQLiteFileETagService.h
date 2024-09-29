#pragma once
#include "FileETagService.h"

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

namespace FileETagService
{

class SQLiteFileETagService : public FileETagService
{
  public:
    SQLiteFileETagService();

    std::string Get(const std::filesystem::path& path)  noexcept override;

    std::string Set(const std::filesystem::path& path)  noexcept override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FileETagService
