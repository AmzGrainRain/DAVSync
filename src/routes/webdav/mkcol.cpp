#include "mkcol.h"
#include <exception>
#include <filesystem>

#include "ConfigReader.h"
#include "http_exceptions.hpp"
#include "logger.hpp"

namespace Routes::WebDAV
{

void MKCOL(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();

    try
    {
        fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (fs::exists(abs_path))
        {
            if (fs::is_directory(abs_path))
            {
                throw ConflictException("The resource already exists");
            }
            throw MethodNotAllowedException("The request method is not allowed for this resource");
        }

        fs::create_directories(abs_path);
    }
    catch (const ConflictException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::conflict);
    }
    catch (const MethodNotAllowedException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::method_not_allowed);
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
