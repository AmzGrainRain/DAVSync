#include "mkcol.h"
#include <exception>
#include <filesystem>
#include <string>

#include "ConfigReader.h"
#include "utils/webdav.h"

namespace Routes::WebDAV
{

void MKCOL(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();

    std::filesystem::path abs_path = utils::webdav::uri_to_absolute(conf.GetWebDavAbsoluteDataPath(), conf.GetWebDavPrefix(), req.get_url());
    if (fs::exists(abs_path))
    {
        if (fs::is_directory(abs_path))
        {
            res.set_status(cinatra::status_type::conflict);
            return;
        }
        res.set_status(cinatra::status_type::method_not_allowed);
        return;
    }

    try {
        fs::create_directories(abs_path);
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
