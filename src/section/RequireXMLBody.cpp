#include "RequireXMLBody.h"

namespace Section
{

// NOLINTNEXTLINE
bool RequireXMLBody::before(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return req.get_header_value("Content-Type").contains("application/xml");
}

// NOLINTNEXTLINE
bool RequireXMLBody::after(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return true;
}

} // namespace Section
