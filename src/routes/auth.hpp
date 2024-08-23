#pragma once

#include "cinatra/coro_http_request.hpp"
#include "cinatra/coro_http_response.hpp"
using namespace cinatra;


namespace Route::Auth
{

using HANDLER_T = void (*)(coro_http_request&, coro_http_response&);

inline constexpr HANDLER_T DigestAuthentication = [](coro_http_request& req, coro_http_response& res) -> void {

};

} // namespace RouteGetList
