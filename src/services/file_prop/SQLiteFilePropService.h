#pragma once
#include "FilePropService.h"

#include <string>

#include <ormpp/dbng.hpp>
#include <ormpp/sqlite.hpp>

namespace FilePropService
{

class SQLiteFilePropService : public FilePropService
{
  public:
    SQLiteFilePropService();

    bool Set(const std::string& path_sha, const PropT& prop) override;

    std::string Get(const std::string& path_sha, const std::string& key) override;

    std::vector<PropT> GetAll(const std::string& path_sha) override;

    bool Remove(const std::string& path_sha, const std::string& key) override;

    bool RemoveAll(const std::string& path_sha) override;

  private:
    ormpp::dbng<ormpp::sqlite> dbng_;
};

} // namespace FilePropService
