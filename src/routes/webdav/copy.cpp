#include "copy.h"
#include <exception>
#include <filesystem>
#include <string_view>

#include "ConfigReader.h"
#include "utils/webdav.h"

namespace Routes::WebDAV
{

void COPY(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();
    const auto uri_to_absolute = [&conf](const std::string_view& uri){
        return utils::webdav::uri_to_absolute(conf.GetWebDavAbsoluteDataPath(), conf.GetWebDavPrefix(), uri);
    };

    // source
    fs::path source_path = uri_to_absolute(req.get_url());
    if (!fs::exists(source_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    // TODO: LOCK

    const std::string_view& dest_header = req.get_header_value("Destination");
    if (dest_header.empty())
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    // copy in place? do nothing
    fs::path dest_path = uri_to_absolute(dest_header);
    if (source_path == dest_path)
    {
        // res.set_status(cinatra::status_type::precondition_failed);
        res.set_status(cinatra::status_type::ok);
        return;
    }

    if (!fs::exists(dest_path))
    {
        try
        {
            if (fs::is_directory(source_path))
                fs::copy(source_path, dest_path, fs::copy_options::recursive);
            else
                fs::copy(source_path, dest_path);
            res.set_status(cinatra::status_type::ok);
        }
        catch (const std::exception& err)
        {
            std::cerr << err.what() << std::endl;
            res.set_status(cinatra::status_type::internal_server_error);
        }

        return;
    }

    const std::string_view& overwite_header = req.get_header_value("Overwrite");
    if (overwite_header.empty())
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    if (overwite_header[0] == 'F')
    {
        res.set_status(cinatra::status_type::precondition_failed);
        return;
    }

    if (overwite_header[0] != 'T')
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    // final
    try {
        if (fs::is_directory(dest_path))
            fs::copy(source_path, dest_path, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        else
            fs::copy(source_path, dest_path, fs::copy_options::overwrite_existing);
        res.set_status(cinatra::status_type::ok);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
