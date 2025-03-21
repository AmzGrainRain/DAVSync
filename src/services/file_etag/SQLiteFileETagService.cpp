#include "SQLiteFileETagService.h"

#include <format>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"


namespace FileETagService
{

SQLiteFileETagService::SQLiteFileETagService()
{
    using namespace ormpp;

    if (const auto& conf = ConfigReader::GetInstance(); !dbng_.connect(utils::path::to_string(conf.GetSQLiteDB()).data(), "ETag"))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileETagTable>(ormpp_key{"path"}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

std::string SQLiteFileETagService::Get(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const auto query_res = dbng_.query_s<FileETagTable>(std::format("path='{}'", path_str));
    if (query_res.size() != 1)
    {
        LOG_ERROR("The database may be damaged.")
        return {""};
    }

    return query_res[0].sha;
}

std::string SQLiteFileETagService::Set(const std::filesystem::path& path) noexcept
{
    std::string path_str = utils::path::to_string(path);
    {
        const std::string where = std::format("path='{}'", path_str);
        const auto query_res = dbng_.query_s<FileETagTable>(where);
        if (!query_res.empty())
        {
            if (!dbng_.delete_records_s<FileETagTable>(where))
            {
                LOG_ERROR("Data deletion failed.")
            }
        }
    }

    std::string sha{};
    if (std::filesystem::is_directory(path))
    {
        sha = utils::sha256(path_str);
    }
    else if (std::filesystem::is_regular_file(path))
    {
        sha = utils::sha256(path);
    }
    else
    {
        LOG_WARN("Unexpected file type.")
        return {""};
    }

    if (dbng_.insert<FileETagTable>({std::move(path_str), sha}) != 1)
    {
        LOG_ERROR("Data insertion error.");
        return {""};
    }

    return sha;
}

} // namespace FileETagService
