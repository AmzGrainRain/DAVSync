#pragma once

#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>

namespace Section
{

struct BasicAuth
{
    bool before(cinatra::coro_http_request& req, cinatra::coro_http_response& res);

    bool after(cinatra::coro_http_request& req, cinatra::coro_http_response& res);
};

} // namespace Section
