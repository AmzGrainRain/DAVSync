#pragma once

#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>
#include <async_simple/coro/Lazy.h>

namespace Routes::WebDAV
{

async_simple::coro::Lazy<void> PUT(cinatra::coro_http_request& req, cinatra::coro_http_response& res);

} // namespace Routes::WebDAV
