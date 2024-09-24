#include "propfind.h"
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <sstream>

#include <pugixml.hpp>
#include <system_error>

#include "ConfigReader.h"
#include "utils/webdav.h"

namespace Routes::WebDAV
{

void PROPFIND(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    const auto& conf = ConfigReader::GetInstance();

    std::filesystem::path abs_path =
        utils::webdav::uri_to_absolute(conf.GetWebDavAbsoluteDataPath(), conf.GetWebDavPrefix(), req.get_url());
    bool is_file = std::filesystem::is_regular_file(abs_path);
    if (!is_file && !std::filesystem::is_directory(abs_path))
    {
        res.set_status(cinatra::status_type::not_found);
        return;
    }

    const auto& depth_header = req.get_header_value("Depth");
    if (depth_header.empty())
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    int8_t depth;
    if (depth_header == "infinity")
    {
        const auto& conf = ConfigReader::GetInstance();
        const auto [ptr, ec] = std::from_chars(depth_header.data(), depth_header.data() + depth_header.size(), depth);
        depth = ec == std::errc() ? 1 : conf.GetWebDavMaxRecurseDepth();
    }
    else
    {
        const auto [ptr, ec] = std::from_chars(depth_header.data(), depth_header.data() + depth_header.size(), depth);
        if (ec != std::errc())
        {
            depth = 1;
        }
    }

    pugi::xml_document xml_doc;
    pugi::xml_node multistatus =
        utils::webdav::generate_multistatus(xml_doc, conf.GetHttpsEnabled(), conf.GetHttpHost());
    utils::webdav::generate_response_list_recurse(multistatus, abs_path, depth);
    std::ostringstream oss;
    xml_doc.save(oss);

    res.add_header("Content-Type", "application/xml; charset=utf-8");
    res.add_header("Allow", is_file
                                ? "OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, COPY, MOVE, LOCK, UNLOCK"
                                : "OPTIONS, GET, HEAD, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK");
    res.set_status(cinatra::status_type::multi_status);
    res.set_content_type<cinatra::resp_content_type::xml>();
    res.set_content(oss.str());
}

} // namespace Routes::WebDAV
