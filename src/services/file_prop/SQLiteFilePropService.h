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
    SQLiteFilePropService();

    bool Set(const std::filesystem::path& path, const PropT& prop) override;

    std::string Get(const std::filesystem::path& path, const std::string& key) override;

    std::vector<PropT> GetAll(const std::filesystem::path& path) override;

    bool Remove(const std::filesystem::path& path, const std::string& key) override;

    bool RemoveAll(const std::filesystem::path& path) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FilePropService
