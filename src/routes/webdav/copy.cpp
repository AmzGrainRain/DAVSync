#include "copy.h"
#include <exception>
#include <filesystem>
#include <string_view>

#include "ConfigReader.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileLockService.h"

static inline std::string GetLockToken(std::string if_header)
{
    std::regex lock_token_pattern(R"(\(<([^>]+)>\))");
    std::smatch match;

    if (std::regex_search(if_header, match, lock_token_pattern))
    {
        return match[1];
    }
    return "";
}

namespace Routes::WebDAV
{

void COPY(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();

    try
    {
        // source
        fs::path source_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!fs::exists(source_path))
        {
            throw NotFoundException("Source not found");
        }

        // if header
        const std::string if_header_str{req.get_header_value("If")};
        std::string lock_token = GetLockToken(if_header_str);

        // is locked?
        static auto& lock_service = FileLock::Service::GetInstance();
        if (lock_service.IsLocked(source_path, true))
        {
            const auto* lock_list = lock_service.GetAllLock(source_path);
            for (const auto& lock : *lock_list)
            {
                if (lock.second->scope == FileLock::LockScope::EXCLUSIVE || lock.second->type == FileLock::LockType::READ)
                {
                    throw LockedException("Source is locked");
                }
            }
        }

        // destination header
        const std::string_view& dest_header = req.get_header_value("Destination");
        if (dest_header.empty())
        {
            throw BadRequestException("Destination header not found");
        }

        // copy in place?
        fs::path dest_path = conf.GetWebDavAbsoluteDataPath(dest_header);
        if (source_path == dest_path)
        {
            throw ConflictException("Source and Destination are the same");
        }

        // destination no exists
        if (!fs::exists(dest_path))
        {
            if (fs::is_directory(source_path))
            {
                fs::copy(source_path, dest_path, fs::copy_options::recursive);
            }
            else
            {
                fs::copy(source_path, dest_path);
            }
            res.set_status(cinatra::status_type::ok);
            return;
        }

        // Overwrite?
        const std::string_view& overwite_header = req.get_header_value("Overwrite");
        if (overwite_header.empty())
        {
            throw BadRequestException("Overwrite header not found");
        }

        if (overwite_header[0] == 'F')
        {
            throw PreconditionFailedException("Destination already exists");
        }

        if (overwite_header[0] != 'T')
        {
            throw BadRequestException("Invalid Overwrite header");
        }

        if (fs::is_directory(dest_path))
        {
            fs::copy(source_path, dest_path, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }
        else
        {
            fs::copy(source_path, dest_path, fs::copy_options::overwrite_existing);
        }
        res.set_status(cinatra::status_type::ok);
    }
    catch (const NotFoundException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::not_found);
    }
    catch (const LockedException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::locked);
    }
    catch (const BadRequestException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::bad_request);
    }
    catch (const ConflictException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::conflict);
    }
    catch (const PreconditionFailedException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::precondition_failed);
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
