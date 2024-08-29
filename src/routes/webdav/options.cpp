#include "options.h"

namespace Routes::WebDAV
{

void OPTIONS(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    using namespace cinatra;

    res.add_header("Allow",
                   "OPTIONS, GET, HEAD, POST, PUT, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK");
    res.add_header("DAV", "1, 2");
    res.add_header("Connection", "close");
    res.set_status(status_type::ok);
}

} // namespace Routes::WebDAV
