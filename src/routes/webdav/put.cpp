#include "put.h"
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>

#include <cinatra/coro_http_connection.hpp>

#include "utils/webdav.h"

namespace Routes::WebDAV
{

async_simple::coro::Lazy<void> PUT(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;

    std::string abs_path = utils::webdav::uri_to_absolute(req.get_url());
    if (fs::exists(abs_path))
    {
        // TODO: LOCK

        if (fs::is_directory(abs_path))
        {
            res.set_status(cinatra::status_type::conflict);
            co_return;
        }

        res.set_status(cinatra::status_type::method_not_allowed);
        co_return;
    }

    if (req.get_content_type() != cinatra::content_type::chunked)
    {
        res.set_status(cinatra::status_type::bad_request);
        co_return;
    }

    try
    {
        std::ofstream ofs(abs_path);
        if (!ofs.is_open())
        {
            throw std::runtime_error(std::format("Unable to open the specified file: {}", abs_path));
        }

        cinatra::chunked_result result{};
        while (true)
        {
            result = co_await req.get_conn()->read_chunked();
            if (result.ec)
                co_return;
            if (result.eof)
                break;

            ofs.write(result.data.data(), result.data.size());
        }

        ofs.flush();
        ofs.close();

        // TODO: compute file SHA256 & save to postgresql


        res.set_status(cinatra::status_type::ok);
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
