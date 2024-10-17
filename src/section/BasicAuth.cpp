#include "BasicAuth.h"

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/string.h"

inline static void RequestVerification(cinatra::coro_http_response& res) noexcept
{
    const auto& conf = ConfigReader::GetInstance();

    std::string header_content = std::format("Basic realm=\"{}\"", conf.GetWebDavRealm());
    res.add_header("WWW-Authenticate", header_content);
    res.set_status(cinatra::status_type::unauthorized);
}

inline static auto ParseAuthorization(const std::string_view& text_view) noexcept -> std::pair<std::string, std::string>
{
    // text_view just like: "Basic dXNlcjpwYXNzd29yZA=="
    std::string encoded_base64{utils::string::split(text_view, ' ')[1]};
    const std::string decoded_base64 = utils::base64_decode(encoded_base64);

    return utils::string::split2pair(decoded_base64, ':');
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

        const auto& conf = ConfigReader::GetInstance();
        const auto client_user = ParseAuthorization(authorization);
        const std::string serverside_user_password = conf.GetWebDavUser(client_user.first);
        if (serverside_user_password.empty() || client_user.second != serverside_user_password)
        {
            RequestVerification(res);
            return false;
        }

        // save username, which plays an important role in the future
        req.set_aspect_data(std::move(client_user.first));

        return true;
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::bad_request);
        return false;
    }
}

bool BasicAuth::after(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return true;
}

} // namespace Section
