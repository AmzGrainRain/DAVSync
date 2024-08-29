#include <exception>
#include <iostream>
#include <string_view>

#include <cinatra/coro_http_request.hpp>  // class cinatra::coro_http_request
#include <cinatra/coro_http_response.hpp> // class cinatra::coro_http_response
#include <cinatra/coro_http_server.hpp>   // class cinatra::coro_http_server
#include <pugixml.hpp>                    // namespace pugixml

#include "section/basic_auth.h"  // struct Section::BasicAuth
#include "section/digest_auth.h" // struct Section::DigestAuth

#include "routes/webdav/copy.h"      // func Routes::WebDAV::COPY
#include "routes/webdav/delete.h"    // func Routes::WebDAV::DEL
#include "routes/webdav/get.h"       // func Routes::WebDAV::GET
#include "routes/webdav/head.h"      // func Routes::WebDAV::HEAD
#include "routes/webdav/lock.h"      // func Routes::WebDAV::LOCK
#include "routes/webdav/mkcol.h"     // func Routes::WebDAV::MKCOL
#include "routes/webdav/move.h"      // func Routes::WebDAV::MOVE
#include "routes/webdav/options.h"   // func Routes::WebDAV::OPTIONS
#include "routes/webdav/post.h"      // func Routes::WebDAV::POST
#include "routes/webdav/propfind.h"  // func Routes::WebDAV::PROPFIND
#include "routes/webdav/proppatch.h" // func Routes::WebDAV::PROPPATCH
#include "routes/webdav/put.h"       // func Routes::WebDAV::PUT
#include "routes/webdav/unlock.h"    // func Routes::WebDAV::UNLOCK

#include "config_reader.h" // class ConfigReader

inline static void use_webdav_service(cinatra::coro_http_server& app, const std::string prefix,
                                      const std::string_view& auth_type)
{
    using namespace cinatra;
    namespace R = Routes::WebDAV;

    if (auth_type == "basic")
    {
        app.set_http_handler<OPTIONS>(prefix, R::OPTIONS, Section::BasicAuth{});
        app.set_http_handler<GET>(prefix, Routes::WebDAV::GET, Section::BasicAuth{});
        app.set_http_handler<HEAD>(prefix, R::HEAD, Section::BasicAuth{});
        app.set_http_handler<POST>(prefix, R::POST, Section::BasicAuth{});
        app.set_http_handler<PUT>(prefix, R::PUT, Section::BasicAuth{});
        app.set_http_handler<DEL>(prefix, R::DEL, Section::BasicAuth{});
        app.set_http_handler<PROPFIND>(prefix, R::PROPFIND, Section::BasicAuth{});
        app.set_http_handler<PROPPATCH>(prefix, R::PROPPATCH, Section::BasicAuth{});
        app.set_http_handler<MKCOL>(prefix, R::MKCOL, Section::BasicAuth{});
        app.set_http_handler<COPY>(prefix, R::COPY, Section::BasicAuth{});
        app.set_http_handler<MOVE>(prefix, R::MOVE, Section::BasicAuth{});
        app.set_http_handler<LOCK>(prefix, R::LOCK, Section::BasicAuth{});
        app.set_http_handler<UNLOCK>(prefix, R::UNLOCK, Section::BasicAuth{});
    }
    else if (auth_type == "digest")
    {
        app.set_http_handler<OPTIONS>(prefix, R::OPTIONS, Section::DigestAuth{});
        app.set_http_handler<GET>(prefix, R::GET, Section::DigestAuth{});
        app.set_http_handler<HEAD>(prefix, R::HEAD, Section::DigestAuth{});
        app.set_http_handler<POST>(prefix, R::POST, Section::DigestAuth{});
        app.set_http_handler<PUT>(prefix, R::PUT, Section::DigestAuth{});
        app.set_http_handler<DEL>(prefix, R::DEL, Section::DigestAuth{});
        app.set_http_handler<PROPFIND>(prefix, R::PROPFIND, Section::DigestAuth{});
        app.set_http_handler<PROPPATCH>(prefix, R::PROPPATCH, Section::DigestAuth{});
        app.set_http_handler<MKCOL>(prefix, R::MKCOL, Section::DigestAuth{});
        app.set_http_handler<COPY>(prefix, R::COPY, Section::DigestAuth{});
        app.set_http_handler<MOVE>(prefix, R::MOVE, Section::DigestAuth{});
        app.set_http_handler<LOCK>(prefix, R::LOCK, Section::DigestAuth{});
        app.set_http_handler<UNLOCK>(prefix, R::UNLOCK, Section::DigestAuth{});
    }
    else if (auth_type == "none")
    {
        app.set_http_handler<OPTIONS>(prefix, R::OPTIONS);
        app.set_http_handler<GET>(prefix, R::GET);
        app.set_http_handler<HEAD>(prefix, R::HEAD);
        app.set_http_handler<POST>(prefix, R::POST);
        app.set_http_handler<PUT>(prefix, R::PUT);
        app.set_http_handler<DEL>(prefix, R::DEL);
        app.set_http_handler<PROPFIND>(prefix, R::PROPFIND);
        app.set_http_handler<PROPPATCH>(prefix, R::PROPPATCH);
        app.set_http_handler<MKCOL>(prefix, R::MKCOL);
        app.set_http_handler<COPY>(prefix, R::COPY);
        app.set_http_handler<MOVE>(prefix, R::MOVE);
        app.set_http_handler<LOCK>(prefix, R::LOCK);
        app.set_http_handler<UNLOCK>(prefix, R::UNLOCK);
    }
    else
    {
        throw std::exception("[settings.ini] Unknow webdav.verification value.");
    }
}

int main()
{
    using namespace cinatra;

    try
    {
        const ConfigReader& conf = ConfigReader::GetInstance();

        coro_http_server app{conf.GetHttpMaxThread(), conf.GetHttpPort(), conf.GetHttpAddress()};
        use_webdav_service(app, conf.GetWebDavRawPrefix(), conf.GetWebDavVerification());

        app.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& res) -> void {
            res.set_content_type<resp_content_type::html>();
            res.set_status_and_content(status_type::ok, "<h1>The server has been started.</h1>");
        });

        return app.sync_start().value();
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}
