#include "propfind.h"

#include <cstdint>
#include <filesystem>
#include <sstream>

#include <pugixml.hpp>
#include <system_error>

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "utils/webdav.h"

namespace Routes::WebDAV
{

void PROPFIND(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;
    const auto& conf = ConfigManager::GetInstance();

    try
    {
        fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        bool is_file = std::filesystem::is_regular_file(abs_path);
        if (!is_file && !std::filesystem::is_directory(abs_path))
        {
            throw NotFoundException("path not found");
        }

        int8_t depth;
        {
            std::string depth_header{req.get_header_value("Depth")};
            if (depth_header.empty())
            {
                throw BadRequestException("Depth header is empty");
            }
            depth = depth_header == "Infinity" ? conf.GetWebDavMaxRecurseDepth() : static_cast<int8_t>(std::stoi(depth_header));
        }

        pugi::xml_document xml_doc;
        pugi::xml_node root = utils::webdav::generate_multistatus_header(xml_doc);
        utils::webdav::generate_response_list_recurse(root, abs_path, depth);
        std::ostringstream oss;
        xml_doc.save(oss);

        res.add_header("Content-Type", "application/xml; charset=utf-8");
        res.add_header("Allow", is_file ? "OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, COPY, MOVE, LOCK, UNLOCK"
                                        : "OPTIONS, GET, HEAD, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK");
        res.set_status(cinatra::status_type::multi_status);
        res.set_content_type<cinatra::resp_content_type::xml>();
        res.set_content(oss.str());
    }
    catch (const NotFoundException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::not_found);
    }
    catch (const BadRequestException& err)
    {
        LOG_INFO(err.what())
        res.set_status(cinatra::status_type::bad_request);
    }
    catch (const std::system_error& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::internal_server_error);
    }
}

} // namespace Routes::WebDAV
