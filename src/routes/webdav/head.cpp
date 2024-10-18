#include "head.h"
#include <filesystem>

#include "ConfigReader.h"
#include "utils/file.h"

namespace Routes::WebDAV
{

void HEAD(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();

    fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
    if (!fs::exists(abs_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    char buffer[100];
    std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", utils::file::get_last_modified(abs_path));

    res.add_header("Content-Type", "application/octet-stream");
    res.add_header("Last-Modified", buffer);
    res.add_header("ETag",  buffer);
    res.set_status(cinatra::status_type::ok);
}

} // namespace Routes::WebDAV
