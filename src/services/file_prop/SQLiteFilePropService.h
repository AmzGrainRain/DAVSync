#pragma once
#include "FilePropService.h"

#include <filesystem>
#include <string>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

namespace FilePropService
{

class SQLiteFilePropService : public FilePropService
{
  public:
    SQLiteFilePropService() noexcept(false);

    bool Set(const std::filesystem::path& path, const PropT& prop) noexcept override;

    std::string Get(const std::filesystem::path& path, const std::string& key) noexcept override;

    std::vector<PropT> GetAll(const std::filesystem::path& path) noexcept override;

    bool Remove(const std::filesystem::path& path, const std::string& key) noexcept override;

    bool RemoveAll(const std::filesystem::path& path) noexcept override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FilePropService
