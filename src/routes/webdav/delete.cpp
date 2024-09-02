#include "delete.h"
#include <exception>
#include <filesystem>
#include <iostream>

#include "utils/webdav.h"

namespace Routes::WebDAV
{

void DEL(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;

    fs::path abs_path = utils::webdav::uri_to_absolute(req.get_url());
    if (!std::filesystem::exists(abs_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    // TODO: LOCK

    try
    {
        if (fs::is_directory(abs_path))
            fs::remove_all(abs_path);
        else
            fs::remove(abs_path);

        res.set_status(cinatra::status_type::ok);
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
