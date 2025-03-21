#include "SQLiteFilePropService.h"

#include <entity.hpp>
#include <filesystem>
#include <format>

#include <vector>

#include "ConfigReader.h"
#include "FilePropService.h"
#include "logger.hpp"
#include "utils/path.h"


namespace FilePropService
{

REGISTER_AUTO_KEY(FilePropTable, id)

SQLiteFilePropService::SQLiteFilePropService() noexcept(false)
{
    using namespace ormpp;

    if (const auto& conf = ConfigReader::GetInstance(); !dbng_.connect(utils::path::to_string(conf.GetSQLiteDB()).data(), "FileProp"))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FilePropTable>(ormpp_auto_key{"id"}, ormpp_unique{{"path"}}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

bool SQLiteFilePropService::Set(const std::filesystem::path& path, const PropT& prop) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    return dbng_.insert<FilePropTable>({path_str, prop.first, prop.second}) == 1;
}

std::string SQLiteFilePropService::Get(const std::filesystem::path& path, const std::string& key) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const auto query_res = dbng_.query_s<FilePropTable>(std::format("path='{}' and key='{}'", path_str, key));
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.");
        return {""};
    }

    return query_res[0].value;
}

std::vector<PropT> SQLiteFilePropService::GetAll(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    std::vector<PropT> props;

    for (const auto query_res = dbng_.query_s<FilePropTable>(std::format("path='{}'", path_str)); const auto& prop : query_res)
    {
        props.emplace_back(prop.key, prop.value);
    }

    return props;
}

bool SQLiteFilePropService::Remove(const std::filesystem::path& path, const std::string& key) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    return dbng_.delete_records_s<FilePropTable>(std::format("path='{}' and key='{}'", path_str, key));
}

bool SQLiteFilePropService::RemoveAll(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    return dbng_.delete_records_s<FilePropTable>(std::format("path='{}'", path_str));
}

} // namespace FilePropService
