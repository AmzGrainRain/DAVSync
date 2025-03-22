#include "get.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <async_simple/coro/SyncAwait.h>
#include <cinatra/coro_http_connection.hpp>
#include <pugixml.hpp>

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileETagServiceFactory.h"

inline async_simple::coro::Lazy<void> SendFile(cinatra::coro_http_response& res, const std::filesystem::path& path)
{
    using namespace cinatra;

    std::ifstream ifs(path, std::ios::binary);
    res.set_format_type(format_type::chunked);

    coro_http_connection* const conn = res.get_conn();
    if (!(co_await conn->begin_chunked()))
    {
        co_return;
    }

    const auto& conf = ConfigManager::GetInstance();
    const size_t BUFFER_SIZE = conf.GetHttpBufferSize();
    std::vector<char> buffer(BUFFER_SIZE);
    size_t readed = 0;
    bool ok;
    while ((readed = ifs.readsome(&buffer[0], BUFFER_SIZE)) > 0)
    {
        buffer.resize(readed);
        ok = co_await conn->write_chunked({buffer.begin(), buffer.end()});
        if (!ok)
        {
            co_return;
        }
    }
    co_return;
}

namespace Routes::WebDAV
{

void GET(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigManager::GetInstance();

    try
    {
        fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (!fs::exists(abs_path))
        {
            throw NotFoundException("File not found.");
        }

        if (fs::is_directory(abs_path))
        {
            throw BadRequestException("Directory not allowed.");
        }

        if (!fs::is_regular_file(abs_path))
        {
            throw BadRequestException("Not a regular file.");
        }

        auto& etag_service = FileETagService::GetService();
        std::string etag = etag_service.Set(abs_path);
        if (etag.empty())
        {
            throw std::runtime_error("Failed to compute ETag.");
        }

        //  There are ONLY TWO SITUATIONS where we need to respond to the client:
        //  1. When the request header does not have "If-None-Match" property.
        //  2. When the request header has "If-None-Match", and its value does
        //     not match the ETag computed by the server.
        const std::string_view& client_etag = req.get_header_value("If-None-Match");
        if (client_etag.empty() || client_etag != etag)
        {
            res.add_header("ETag", etag);
            async_simple::coro::syncAwait(SendFile(res, abs_path));
            return;
        }

        // ETag matched
        res.set_status(cinatra::status_type::not_modified);
    }
    catch (const NotFoundException& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::not_found);
    }
    catch (const BadRequestException& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::bad_request);
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
