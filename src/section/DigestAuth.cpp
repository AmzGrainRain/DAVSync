#include "DigestAuth.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ConfigReader.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/map.hpp"
#include "utils/string.h"

inline static void RequestVerification(cinatra::coro_http_response& res) noexcept
{
    const auto& conf = ConfigReader::GetInstance();

    const std::string header_content =
        std::format("Digest realm=\"{}\", qio=\"auth\", nonce=\"{}\", opaque=\"{}\", algorithm=\"SHA-256\"",
                    conf.GetWebDavRealm(), utils::generate_unique_key(), utils::generate_unique_key());
    res.add_header("WWW-Authenticate", header_content);
    res.set_status(cinatra::status_type::unauthorized);
}

inline static auto ParseAuthorizationHeader(const std::string_view& text_view) noexcept
    -> std::unordered_map<std::string, std::string>
{
    std::unordered_map<std::string, std::string> map{};
    std::string text{text_view};
    utils::string::replace(text, "Digest ", "");

    for (const auto& kv_str : utils::string::split(text, ','))
    {
        auto pair = utils::string::split2pair(kv_str, '=');
        if (!pair.first.empty())
        {
            map.insert(std::move(pair));
        }
    }

    return map;
}

inline static std::string ComputeHA1(const std::string& username) noexcept
{
    const auto& conf = ConfigReader::GetInstance();
    const std::string password = conf.GetWebDavUser(username);
    if (password.empty())
    {
        return {""};
    }

    // username:realm:password
    return utils::sha256(std::format("{}:{}:{}", username, conf.GetWebDavRealm(), password));
}

inline static std::string ComputeHA2(const std::string_view& method, const std::string_view& uri) noexcept
{
    // method:uri
    return utils::sha256(std::format("{}:{}", method, uri));
}

namespace Section
{

bool DigestAuth::before(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    try
    {
        const std::string_view& authorization = req.get_header_value("Authorization");
        if (authorization.empty())
        {
            RequestVerification(res);
            return false;
        }

        if (!authorization.starts_with("Digest "))
        {
            res.set_status(cinatra::status_type::forbidden);
            return false;
        }

        auto map = ParseAuthorizationHeader(authorization);
        if (!utils::map::all_exist(map, {"username", "realm", "nonce", "nc", "conce", "response"}))
        {
            RequestVerification(res);
            return false;
        }

        auto username = map.find("username");
        auto nonce = map.find("nonce");
        auto nc = map.find("nc");
        auto conce = map.find("conce");
        auto qop = map.find("qop");

        const std::string HA1 = ComputeHA1(username->second);
        if (HA1.empty())
        {
            RequestVerification(res);
            return false;
        }

        const std::string HA2 = ComputeHA2(req.get_method(), req.get_url());

        // HA1:nonce:nc:conce:qop:HA2
        const std::string RESPONSE = utils::sha256(
            std::format("{}:{}:{}:{}:{}:{}", HA1, nonce->second, nc->second, conce->second, qop->second, HA2));
        if (map.find("response")->second != RESPONSE)
        {
            RequestVerification(res);
            return false;
        }
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status(cinatra::status_type::bad_request);
        return false;
    }

    return true;
}

bool DigestAuth::after(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    return true;
}

} // namespace Section
