#include <exception>
#include <locale>

#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>
#include <cinatra/coro_http_server.hpp>
#include <pugixml.hpp>

#include "section/BasicAuth.h"
#include "section/DigestAuth.h"

#include "routes/webdav/copy.h"
#include "routes/webdav/delete.h"
#include "routes/webdav/get.h"
#include "routes/webdav/head.h"
#include "routes/webdav/lock.h"
#include "routes/webdav/mkcol.h"
#include "routes/webdav/move.h"
#include "routes/webdav/options.h"
#include "routes/webdav/post.h"
#include "routes/webdav/propfind.h"
#include "routes/webdav/proppatch.h"
#include "routes/webdav/put.h"
#include "routes/webdav/unlock.h"

#include "logger.hpp"
#include "ConfigReader.h"

int main()
{
    namespace R = Routes::WebDAV;
    using namespace cinatra;

    std::locale::global(std::locale("en_US.UTF-8"));
    std::cout.imbue(std::locale());

    try
    {
        const ConfigReader& conf = ConfigReader::GetInstance();
        coro_http_server app{conf.GetHttpMaxThread(), conf.GetHttpPort(), conf.GetHttpAddress()};
        const std::string& verify = conf.GetWebDavVerification();
        const std::string& webdav_prefix = conf.GetWebDavRoutePrefix();

        if (verify == "basic")
        {
            app.set_http_handler<OPTIONS>(webdav_prefix, R::OPTIONS, Section::BasicAuth{});
            app.set_http_handler<GET>(webdav_prefix, R::GET, Section::BasicAuth{});
            app.set_http_handler<HEAD>(webdav_prefix, R::HEAD, Section::BasicAuth{});
            app.set_http_handler<POST>(webdav_prefix, R::POST, Section::BasicAuth{});
            app.set_http_handler<PUT>(webdav_prefix, R::PUT, Section::BasicAuth{});
            app.set_http_handler<DEL>(webdav_prefix, R::DEL, Section::BasicAuth{});
            app.set_http_handler<PROPFIND>(webdav_prefix, R::PROPFIND, Section::BasicAuth{});
            app.set_http_handler<PROPPATCH>(webdav_prefix, R::PROPPATCH, Section::BasicAuth{});
            app.set_http_handler<MKCOL>(webdav_prefix, R::MKCOL, Section::BasicAuth{});
            app.set_http_handler<COPY>(webdav_prefix, R::COPY, Section::BasicAuth{});
            app.set_http_handler<MOVE>(webdav_prefix, R::MOVE, Section::BasicAuth{});
            app.set_http_handler<LOCK>(webdav_prefix, R::LOCK, Section::BasicAuth{});
            app.set_http_handler<UNLOCK>(webdav_prefix, R::UNLOCK, Section::BasicAuth{});
        }
        else if (verify == "digest")
        {
            app.set_http_handler<OPTIONS>(webdav_prefix, R::OPTIONS, Section::DigestAuth{});
            app.set_http_handler<GET>(webdav_prefix, R::GET, Section::DigestAuth{});
            app.set_http_handler<HEAD>(webdav_prefix, R::HEAD, Section::DigestAuth{});
            app.set_http_handler<POST>(webdav_prefix, R::POST, Section::DigestAuth{});
            app.set_http_handler<PUT>(webdav_prefix, R::PUT, Section::DigestAuth{});
            app.set_http_handler<DEL>(webdav_prefix, R::DEL, Section::DigestAuth{});
            app.set_http_handler<PROPFIND>(webdav_prefix, R::PROPFIND, Section::DigestAuth{});
            app.set_http_handler<PROPPATCH>(webdav_prefix, R::PROPPATCH, Section::DigestAuth{});
            app.set_http_handler<MKCOL>(webdav_prefix, R::MKCOL, Section::DigestAuth{});
            app.set_http_handler<COPY>(webdav_prefix, R::COPY, Section::DigestAuth{});
            app.set_http_handler<MOVE>(webdav_prefix, R::MOVE, Section::DigestAuth{});
            app.set_http_handler<LOCK>(webdav_prefix, R::LOCK, Section::DigestAuth{});
            app.set_http_handler<UNLOCK>(webdav_prefix, R::UNLOCK, Section::DigestAuth{});
        }
        else
        {
            app.set_http_handler<OPTIONS>(webdav_prefix, R::OPTIONS);
            app.set_http_handler<GET>(webdav_prefix, R::GET);
            app.set_http_handler<HEAD>(webdav_prefix, R::HEAD);
            app.set_http_handler<POST>(webdav_prefix, R::POST);
            app.set_http_handler<PUT>(webdav_prefix, R::PUT);
            app.set_http_handler<DEL>(webdav_prefix, R::DEL);
            app.set_http_handler<PROPFIND>(webdav_prefix, R::PROPFIND);
            app.set_http_handler<PROPPATCH>(webdav_prefix, R::PROPPATCH);
            app.set_http_handler<MKCOL>(webdav_prefix, R::MKCOL);
            app.set_http_handler<COPY>(webdav_prefix, R::COPY);
            app.set_http_handler<MOVE>(webdav_prefix, R::MOVE);
            app.set_http_handler<LOCK>(webdav_prefix, R::LOCK);
            app.set_http_handler<UNLOCK>(webdav_prefix, R::UNLOCK);
            LOG_WARN("It seems that you have not set any security verification for the webdav server, which means that "
                     "anyone can directly modify your files without any verification.")
        }

        app.set_http_handler<GET>("/", [](coro_http_request& req, coro_http_response& res) -> void {
            res.set_content_type<resp_content_type::html>();
            res.set_status_and_content(status_type::ok, "<h1>The server has been started.</h1>");
        });

        LOG_INFO_FMT("Server running at {}://{}:{}", conf.GetHttpsEnabled() ? "https" : "http", conf.GetHttpHost(),
                     conf.GetHttpsEnabled() ? conf.GetHttpsPort() : conf.GetHttpPort());

        return app.sync_start().value();
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        return -1;
    }
}
