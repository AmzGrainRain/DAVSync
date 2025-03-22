#include "unlock.h"

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileLockService.h"

namespace Routes::WebDAV
{

void UNLOCK(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    static const auto& conf = ConfigManager::GetInstance();
    const std::string username = req.get_aspect_data()[0];

    try
    {
        std::filesystem::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!fs::exists(abs_path))
        {
            throw NotFoundException("The specified file does not exist");
        }

        static auto& lock_service = FileLock::Service::GetInstance();

        if (!lock_service.Unlock(abs_path, username))
        {
            throw ForbiddenException("The specified file is not locked by the specified user or is already unlocked");
        }

        // TODO: XML response
        res.set_status(cinatra::status_type::ok);
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
