#include "BasicAuth.h"

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/string.h"

inline void RequestVerification(cinatra::coro_http_response& res) noexcept
{
    const auto& conf = ConfigReader::GetInstance();

    const std::string header_content = std::format("Basic realm=\"{}\"", conf.GetWebDavRealm());
    res.add_header("WWW-Authenticate", header_content);
    res.set_status(cinatra::status_type::unauthorized);
}

inline auto ParseAuthorization(const std::string_view& text_view) noexcept -> std::pair<std::string, std::string>
{
    // text_view just like: "Basic dXNlcjpwYXNzd29yZA=="
    const std::string encoded_base64{utils::string::split(text_view, ' ')[1]};
    const std::string decoded_base64 = utils::base64_decode(encoded_base64);

    return utils::string::split2pair(decoded_base64, ':');
}

namespace Section
{
// NOLINTNEXTLINE
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
        const auto& [user, user_credit] = ParseAuthorization(authorization);
        if (const std::string serverside_user_credit = conf.GetWebDavUser(user);
            serverside_user_credit.empty() || user_credit != serverside_user_credit)
        {
            RequestVerification(res);
            return false;
        }

        // save username, which plays an important role in the future
        req.set_aspect_data(user);

        return true;
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::bad_request);
        return false;
    }
}

// NOLINTNEXTLINE
bool BasicAuth::after(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return true;
}

} // namespace Section
