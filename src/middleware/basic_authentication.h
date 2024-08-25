#pragma once

#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>

using namespace cinatra;

namespace Middleware
{

struct BasicAuthentication
{
    bool before(coro_http_request& req, coro_http_response& res);

    bool after(coro_http_request& req, coro_http_response& res);
};

} // namespace Middleware
