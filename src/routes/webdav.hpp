#pragma once

#include "cinatra/coro_http_request.hpp"
#include "cinatra/coro_http_response.hpp"
using namespace cinatra;

namespace Route::WebDav
{

using HANDLER_T = void (*)(coro_http_request&, coro_http_response&);

inline constexpr HANDLER_T PROPFIND = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T PROPPATCH = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T MKCOL = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T COPY = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T MOVE = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T LOCK = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

inline constexpr HANDLER_T UNLOCK = [](coro_http_request& req, coro_http_response& res) -> void {
    res.set_status(status_type::no_content);
};

} // namespace Route::Dav
