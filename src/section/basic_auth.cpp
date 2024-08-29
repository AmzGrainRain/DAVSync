#include "basic_auth.h"

#include <format>
#include <vector>

#include <cinatra/utils.hpp>

#include "config_reader.h"
#include "utils/string.h"

inline static void RequestVerification(cinatra::coro_http_response& res) noexcept
{
    const auto& conf = ConfigReader::GetInstance();

    std::string header_content = std::format("Basic realm=\"{}\"", conf.GetWebDavRealm());
    res.add_header("WWW-Authenticate", header_content);
    res.set_status(cinatra::status_type::unauthorized);
}

inline static std::string ParseAuthorization(const std::string_view& text_view) noexcept
{
    std::string text{text_view};
    return utils::string::split(text, " ")[1];
}

namespace Section
{

bool BasicAuth::before(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    try
    {
        const std::string_view& authorization = req.get_header_value("Authorization");
        if (authorization.empty())
        {
            RequestVerification(res);
            return false;
        }

        if (!authorization.starts_with("Basic "))
        {
            res.set_status(cinatra::status_type::forbidden);
            return false;
        }

        auto client_base64 = ParseAuthorization(authorization);
        const auto& conf = ConfigReader::GetInstance();
        auto server_base64 =
            cinatra::base64_encode(std::format("{}:{}", conf.GetUserAccount(), conf.GetUserPassword()));

        if (client_base64 != server_base64)
        {
            RequestVerification(res);
            return false;
        }
    }
    catch (const std::exception& err)
    {
        std::cout << std::format("[{}:{}] {}", __FILE__, __LINE__, err.what());
        res.set_status(cinatra::status_type::bad_request);
        return false;
    }

    return true;
}

bool BasicAuth::after(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return true;
}

} // namespace Section
