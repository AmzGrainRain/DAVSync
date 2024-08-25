#include <cinatra/coro_http_server.hpp>
#include <pugixml.hpp>
#include <string_view>

#include "cinatra/coro_http_request.hpp"
#include "cinatra/coro_http_response.hpp"

#include "config_reader/config_reader.h"
#include "middleware/basic_authentication.h"
#include "middleware/digest_authentication.h"
#include "routes/webdav.h"

using namespace cinatra;

int main()
{
    const auto& conf = ConfigReader::GetInstance();
    coro_http_server app{conf.GetHttpMaxThread(), conf.GetHttpPort(), conf.GetHttpHost()};

    // Hello World
    app.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& res) -> void {
        res.set_content_type<resp_content_type::html>();
        res.set_status_and_content(status_type::ok, "<h1>Hello World</h1>");
    });

    const std::string_view dav_auto_type = conf.GetWebDavVerification();
    std::string WebDAVPrefix = conf.GetWebDavPrefix();

    app.set_http_handler<HEAD, OPTIONS, GET, PUT, DEL, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK>(
            WebDAVPrefix, Routes::WebDAV::MainHandler, Middleware::DigestAuthentication{});

    // if (dav_auto_type == "Basic")
    // {
    //     app.set_http_handler<HEAD, OPTIONS, GET, PUT, DEL, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK>(
    //         WebDAVPrefix, Routes::WebDAV::MainHandler, Middleware::BasicAuthentication{});
    // }
    // else if (dav_auto_type == "Digest")
    // {
    //     app.set_http_handler<HEAD, OPTIONS, GET, PUT, DEL, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK>(
    //         WebDAVPrefix, Routes::WebDAV::MainHandler, Middleware::DigestAuthentication{});
    // }
    // else
    // {
    //     app.set_http_handler<HEAD, OPTIONS, GET, PUT, DEL, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK>(
    //         WebDAVPrefix, Routes::WebDAV::MainHandler);
    // }

    return app.sync_start().value();
}
