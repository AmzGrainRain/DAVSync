#pragma once

#include "cinatra/coro_http_request.hpp"
#include "cinatra/coro_http_response.hpp"
#include "cinatra/define.h"
using namespace cinatra;

namespace Routes
{

using HANDLER_T = void (*)(coro_http_request&, coro_http_response&);

}

namespace Routes::WebDAV
{

inline HANDLER_T MainHandler = [](coro_http_request& req, coro_http_response& res) -> void {
    switch (method_type(req.get_method())) {
        case http_method::GET:
            // TODO
            break;
        case http_method::HEAD:
            // TODO
            break;
        case http_method::POST:
            // TODO
            break;
        case http_method::PUT:
            // TODO
            break;
        case http_method::DEL:
            // TODO
            break;
        case http_method::OPTIONS:
            // TODO
            break;
        case http_method::PROPFIND:
            // TODO
            break;
        case http_method::PROPPATCH:
            // TODO
            break;
        case http_method::MKCOL:
            // TODO
            break;
        case http_method::COPY:
            // TODO
            break;
        case http_method::MOVE:
            // TODO
            break;
        case http_method::LOCK:
            // TODO
            break;
        case http_method::UNLOCK:
            // TODO
            break;
        // case http_method::ACL:
        //     break;
        // case http_method::SEARCH:
        //     break;
        // case http_method::REPORT:
        //     break;
        default:
            res.set_status(status_type::bad_request);
    }
};

} // namespace Routes::WebDav
