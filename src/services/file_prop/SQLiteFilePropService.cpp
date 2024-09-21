#include "SQLiteFilePropService.h"

#include <format>

#include <spdlog/spdlog.h>
#include <vector>

#include "ConfigReader.h"
#include "FilePropService.h"

namespace FilePropService
{

REGISTER_AUTO_KEY(FilePropTable, id)
REFLECTION(FilePropTable, sha, key, value, id)

SQLiteFilePropService::SQLiteFilePropService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(conf.GetSQLiteDB()))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FilePropTable>(ormpp_auto_key{"id"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool SQLiteFilePropService::Set(const std::string& path_sha, const PropT& prop)
{
    return dbng_.insert<FilePropTable>({path_sha, prop.first, prop.second}) == 1;
}

std::string SQLiteFilePropService::Get(const std::string& path_sha, const std::string& key)
{
    auto query_res = dbng_.query_s<FilePropTable>(std::format("sha='{}' and key='{}'", path_sha, key));
    if (query_res.size() != 1)
    {
        spdlog::error("The database may be damaged.");
        return {""};
    }

    return query_res[0].value;
}

std::vector<PropT> SQLiteFilePropService::GetAll(const std::string& path_sha)
{
    std::vector<PropT> props;

    auto query_res = dbng_.query_s<FilePropTable>(std::format("sha='{}'", path_sha));
    for (const auto& prop : query_res)
    {
        props.push_back({prop.key, prop.value});
    }

    return props;
}

bool SQLiteFilePropService::Remove(const std::string& path_sha, const std::string& key)
{
    return dbng_.delete_records_s<FilePropTable>(std::format("sha='{}' and key='{}'", path_sha, key));
}

bool SQLiteFilePropService::RemoveAll(const std::string& path_sha)
{
    return dbng_.delete_records_s<FilePropTable>(std::format("sha='{}'", path_sha));
}

} // namespace FilePropService
