#pragma once

#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>

namespace Routes::WebDAV
{

void PUT(cinatra::coro_http_request& req, cinatra::coro_http_response& res);

} // namespace Routes::WebDAV
