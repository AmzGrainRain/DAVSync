#include "proppatch.h"
#include <cstring>
#include <exception>
#include <iostream>

#include <pugixml.hpp>

namespace Routes::WebDAV
{

void PROPPATCH(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    const std::string body{req.get_body()};
    if (body.empty())
    {
        res.set_status(cinatra::status_type::bad_request);
        return;
    }

    try {
        pugi::xml_document doc;
        if (!doc.load_string(body.data()))
        {
            res.set_status(cinatra::status_type::bad_request);
            return;
        }

        pugi::xml_node root_node = doc.child("D:propertyupdate");
        if (root_node.empty())
        {
            res.set_status(cinatra::status_type::bad_request);
            return;
        }

        for (auto prop_node : root_node.children()) {
            if (std::strcmp(prop_node.name(), "D:set") == 0)
            {
                // TODO
            }
            else if (std::strcmp(prop_node.name(), "D:remove") == 0)
            {
                // TODO
            }
        }

        res.set_status(cinatra::status_type::ok);
        return;
    } catch (const std::exception& err) {
        std::cout << err.what() << std::endl;
        res.set_status(cinatra::status_type::internal_server_error);
        return;
    }
}

} // namespace Routes::WebDAV
