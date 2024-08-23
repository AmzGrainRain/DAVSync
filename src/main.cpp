#include <cinatra/coro_http_server.hpp>
#include <pugixml.hpp>

#include "cinatra/coro_http_request.hpp"
#include "cinatra/coro_http_response.hpp"

#include "config_reader/config_reader.h"
#include "middleware/digest_authentication.h"
#include "routes/webdav.hpp"

using namespace cinatra;


int main()
{
    ConfigReader conf{};
    coro_http_server app{conf.GetHttpMaxThread(), conf.GetHttpPort(), conf.GetHttpHost()};

    // Hello World
    app.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& res) -> void {
        res.set_content_type<resp_content_type::html>();
        res.set_status_and_content(status_type::ok, "<h1>Hello World</h1>");
    });

    // TODO: DAV Services
    app.set_http_handler<PROPFIND>("/webdav", Route::WebDav::PROPFIND, Middleware::DigestAuthentication{});
    app.set_http_handler<PROPPATCH>("/webdav", Route::WebDav::PROPPATCH, Middleware::DigestAuthentication{});
    app.set_http_handler<MKCOL>("/webdav", Route::WebDav::MKCOL, Middleware::DigestAuthentication{});
    app.set_http_handler<COPY>("/webdav", Route::WebDav::COPY, Middleware::DigestAuthentication{});
    app.set_http_handler<MOVE>("/webdav", Route::WebDav::MOVE, Middleware::DigestAuthentication{});
    app.set_http_handler<LOCK>("/webdav", Route::WebDav::LOCK, Middleware::DigestAuthentication{});
    app.set_http_handler<UNLOCK>("/webdav", Route::WebDav::UNLOCK, Middleware::DigestAuthentication{});

    return app.sync_start().value();
}
