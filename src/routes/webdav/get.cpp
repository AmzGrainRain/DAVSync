#include "get.h"

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <PicoSHA2/picosha2.h>
#include <async_simple/coro/SyncAwait.h>
#include <cinatra/coro_http_connection.hpp>
#include <pugixml.hpp>

#include "config_reader.h"
#include "utils/webdav.h"

inline std::string GenerateMultiStatus(const std::filesystem::path& dir)
{
    pugi::xml_document xml_doc;
    utils::webdav::generate_multistatus(xml_doc);
    utils::webdav::generate_response_list(xml_doc, dir);
    std::ostringstream oss;
    xml_doc.save(oss);
    return oss.str();
}

std::mutex Mutex_Routes_WebDAV_IMPL_ComputeETag;
inline std::optional<std::string> ComputeETag(const std::filesystem::path& file_path)
{
    // for thread safe
    std::lock_guard<std::mutex> LOCK(Mutex_Routes_WebDAV_IMPL_ComputeETag);

    std::ifstream ifs(file_path, std::ios::binary);
    if (!ifs.is_open())
    {
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(picosha2::k_digest_size);
    picosha2::hash256(ifs, buffer.begin(), buffer.end());
    return picosha2::bytes_to_hex_string(buffer.begin(), buffer.end());
}

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

    const auto& conf = ConfigReader::GetInstance();
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
    using namespace cinatra;

    std::filesystem::path abs_path = utils::webdav::uri_to_absolute(req.get_url());

    if (!std::filesystem::exists(abs_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    if (std::filesystem::is_directory(abs_path))
    {
        // std::string xml = GenerateMultiStatus(abs_path);
        // res.set_content_type<resp_content_type::xml>();
        // res.set_status_and_content(status_type::ok, xml, content_encoding::gzip);
        // return;
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    if (std::filesystem::is_regular_file(abs_path))
    {
        const auto& client_etag = req.get_header_value("If-None-Match");
        auto server_etag = ComputeETag(abs_path);
        if (!server_etag.has_value())
        {
            res.set_status(status_type::internal_server_error);
            return;
        }

        if (client_etag.empty() || client_etag != server_etag.value())
        {
            res.add_header("ETag", server_etag.value());
            async_simple::coro::syncAwait(SendFile(res, abs_path));
            return;
        }

        res.set_status(status_type::not_modified);
        return;
    }

    res.set_status(status_type::internal_server_error);
}

} // namespace Routes::WebDAV
