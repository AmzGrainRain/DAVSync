#include "get.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <async_simple/coro/SyncAwait.h>
#include <cinatra/coro_http_connection.hpp>
#include <pugixml.hpp>

#include "ConfigReader.h"
#include "utils/webdav.h"

inline std::string GenerateMultiStatus(const std::filesystem::path& dir)
{
    const auto& conf = ConfigReader::GetInstance();
    pugi::xml_document xml_doc;
    utils::webdav::generate_multistatus(xml_doc, conf.GetSSLEnabled(), conf.GetHttpHost());
    utils::webdav::generate_response_list(xml_doc, dir);
    std::ostringstream oss;
    xml_doc.save(oss);
    return oss.str();
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
    namespace fs = std::filesystem;
    const auto& conf = ConfigReader::GetInstance();

    fs::path abs_path = utils::webdav::uri_to_absolute(conf.GetWebDavAbsoluteDataPath(), conf.GetWebDavPrefix(), req.get_url());
    if (!fs::exists(abs_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    // TODO: LOCK

    if (fs::is_directory(abs_path))
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    if (!fs::is_regular_file(abs_path))
    {
        res.set_status(cinatra::status_type::internal_server_error);
        return;
    }

    auto server_etag = utils::webdav::compute_etag(abs_path);
    if (!server_etag)
    {
        res.set_status(cinatra::status_type::internal_server_error);
        return;
    }

    //  There are ONLY TWO SITUATIONS where we need to respond to the client:
    //  1. When the request header does not have "If-None-Match" property.
    //  2. When the request header has "If-None-Match", and its value does
    //     not match the ETag computed by the server.
    const std::string_view& client_etag = req.get_header_value("If-None-Match");
    if (client_etag.empty() || client_etag != server_etag.value())
    {
        res.add_header("ETag", server_etag.value());
        async_simple::coro::syncAwait(SendFile(res, abs_path));
        return;
    }

    // ETag matched
    res.set_status(cinatra::status_type::not_modified);
}

} // namespace Routes::WebDAV
