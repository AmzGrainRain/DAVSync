#include "move.h"
#include <filesystem>

#include "ConfigReader.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileLockService.h"

namespace Routes::WebDAV
{

void MOVE(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    static const auto& conf = ConfigReader::GetInstance();

    try
    {
        fs::path source_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!fs::exists(source_path))
        {
            throw NotFoundException("File not found");
        }

        static auto& lock_service = FileLock::Service::GetInstance();
        if (lock_service.IsLocked(source_path))
        {
            throw LockedException("File is locked");
        }

        const std::string_view& dest_header = req.get_header_value("Destination");
        if (dest_header.empty())
        {
            throw BadRequestException("Destination header is empty");
        }

        fs::path dest_path = conf.GetWebDavAbsoluteDataPath(dest_header);
        if (source_path == dest_path)
        {
            throw BadRequestException("Source and Destination are the same");
        }

        if (!fs::exists(dest_path))
        {
            if (fs::is_directory(source_path))
            {
                fs::copy(source_path, dest_path, fs::copy_options::recursive);
                fs::remove_all(source_path);
            }
            else
            {
                fs::copy(source_path, dest_path);
                fs::remove(source_path);
            }
            res.set_status(cinatra::status_type::ok);
            return;
        }

        const std::string_view& overwite_header = req.get_header_value("Overwrite");
        if (overwite_header.empty())
        {
            throw BadRequestException("Overwrite header is empty");
        }

        if (overwite_header[0] == 'F')
        {
            throw BadRequestException("Overwrite header is false");
        }

        if (overwite_header[0] != 'T')
        {
            throw BadRequestException("Invalid Overwrite header");
        }

        if (fs::is_directory(dest_path))
        {
            fs::copy(source_path, dest_path, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            fs::remove_all(source_path);
        }
        else
        {
            fs::copy(source_path, dest_path, fs::copy_options::overwrite_existing);
            fs::remove(source_path);
        }
        res.set_status(cinatra::status_type::ok);
    }
    catch (const NotFoundException& err)
    {
        LOG_INFO(err.what())
        res.set_status_and_content_view(cinatra::status_type::not_found, err.what());
    }
    catch (const LockedException& err)
    {
        LOG_INFO(err.what())
        res.set_status_and_content_view(cinatra::status_type::locked, err.what());
    }
    catch (const BadRequestException& err)
    {
        LOG_INFO(err.what())
        res.set_status_and_content_view(cinatra::status_type::bad_request, err.what());
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status_and_content_view(cinatra::status_type::internal_server_error, err.what());
    }
}

} // namespace Routes::WebDAV
