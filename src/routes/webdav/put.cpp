#include "put.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <cinatra/coro_http_connection.hpp>

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileETagServiceFactory.h"
#include "services/FileLockService.h"

namespace Routes::WebDAV
{

async_simple::coro::Lazy<void> PUT(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigManager::GetInstance();

    try
    {
        std::filesystem::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!fs::exists(abs_path))
        {
            throw NotFoundException("The specified file does not exist");
        }

        if (fs::is_directory(abs_path))
        {
            throw ConflictException("The specified path is a directory");
        }

        if (req.get_content_type() != cinatra::content_type::chunked)
        {
            throw BadRequestException("Only chunked transfer encoding is allowed for this resource");
        }

        static auto& lock_service = FileLock::Service::GetInstance();
        const auto* lock_list = lock_service.GetAllLock(abs_path);
        if (lock_list != nullptr)
        {
            for (const auto& lock : *lock_list)
            {
                if (lock.second->type == FileLock::LockType::WRITE || lock.second->scope == FileLock::LockScope::EXCLUSIVE)
                {
                    throw LockedException("The specified file is locked");
                }
            }
        }

        std::ofstream ofs(abs_path);
        if (!ofs.is_open())
        {
            throw std::runtime_error("Unable to open the specified file");
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

        // update etag
        static auto& etag_service = FileETagService::GetService();
        etag_service.Set(abs_path);

        res.set_status(cinatra::status_type::ok);
    }
    catch (const NotFoundException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::not_found);
    }
    catch (const ConflictException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::conflict);
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
