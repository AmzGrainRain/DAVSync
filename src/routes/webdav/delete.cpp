#include "delete.h"
#include <exception>
#include <filesystem>

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "services/FileLockService.h"
#include "logger.hpp"

namespace Routes::WebDAV
{

void DEL(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    static const auto& conf = ConfigManager::GetInstance();

    try
    {
        fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!std::filesystem::exists(abs_path))
        {
            throw BadRequestException("File not found");
        }

        static auto& lock_service = FileLock::Service::GetInstance();
        if (lock_service.IsLocked(abs_path))
        {
            throw LockedException("File is locked");
        }

        if (fs::is_directory(abs_path))
            fs::remove_all(abs_path);
        else
            fs::remove(abs_path);
        res.set_status(cinatra::status_type::ok);
    }
    catch (const BadRequestException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::bad_request);
    }
    catch (const LockedException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::locked);
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
