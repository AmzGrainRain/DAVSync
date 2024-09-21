#include "SQLiteFileETagService.h"

#include <spdlog/spdlog.h>

#include "ConfigReader.h"
#include "FileETagService.h"
#include "utils.h"
#include "utils/path.h"

namespace FileETagService
{

REGISTER_AUTO_KEY(FileETagTable, id)
REFLECTION(FileETagTable, path, path_sha, sha)

SQLiteFileETagService::SQLiteFileETagService()
{
    using namespace ormpp;
    const auto& conf = ConfigReader::GetInstance();

    if (!dbng_.connect(conf.GetSQLiteDB()))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }

    if (!dbng_.create_datatable<FileETagTable>(ormpp_auto_key{"id"}, ormpp_unique{{"path_sha"}}))
    {
        throw std::runtime_error(dbng_.get_last_error());
    }
}

std::string SQLiteFileETagService::Get(const std::string& path_sha)
{
    auto query_res = dbng_.query_s<FileETagTable>(std::format("path_sha='{}'", path_sha));
    if (query_res.size() != 1)
    {
        spdlog::error("The database may be damaged.");
        return {""};
    }

    return query_res[0].sha;
}

bool SQLiteFileETagService::Set(const std::filesystem::path& file)
{
    std::string path_sha = utils::sha256(utils::path::to_string(file));
    std::string file_sha = utils::sha256(file);
    return dbng_.insert<FileETagTable>({std::move(path_sha), std::move(file_sha)}) == 1;
}

} // namespace FileETagService
