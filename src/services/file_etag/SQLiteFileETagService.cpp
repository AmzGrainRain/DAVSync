#include "SQLiteFileETagService.h"

#include <format>
#include <spdlog/spdlog.h>

#include "ConfigReader.h"
#include "FileETagService.h"
#include "utils.h"
#include "utils/path.h"

namespace FileETagService
{

REGISTER_AUTO_KEY(FileETagTable, id)
REFLECTION(FileETagTable, path, sha, id)

SQLiteFileETagService::SQLiteFileETagService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(conf.GetSQLiteDB()))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileETagTable>(ormpp_auto_key{"id"}, ormpp_unique{{"path"}}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

std::string SQLiteFileETagService::Get(const std::filesystem::path& path)
{
    const std::string path_str = utils::path::to_string(path);
    const auto query_res = dbng_.query_s<FileETagTable>(std::format("path='{}'", path_str));
    if (query_res.size() != 1)
    {
        spdlog::error("The database may be damaged.");
        return {""};
    }

    return query_res[0].sha;
}

bool SQLiteFileETagService::Set(const std::filesystem::path& path)
{
    std::string path_str = utils::path::to_string(path);
    {
        const std::string where = std::format("path='{}'", path_str);
        const auto query_res = dbng_.query_s<FileETagTable>(where);
        if (!query_res.empty())
        {
            if (!dbng_.delete_records_s<FileETagTable>(where))
            {
                spdlog::warn("Data deletion failed.");
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
        throw std::runtime_error("Unexpected file type.");
    }

    return dbng_.insert<FileETagTable>({std::move(path_str), std::move(sha)}) == 1;
}

} // namespace FileETagService
