#include "digest_authentication.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "config_reader/config_reader.h"
#include "utils.h"
#include "utils/map_utils.hpp"
#include "utils/string_utils.h"

inline static void RequestVerification(coro_http_response& res)
{
    const auto& conf = ConfigReader::GetInstance();

    std::string header_content =
        std::format("Digest realm=\"{}\", qio=\"auth\", nonce=\"{}\", opaque=\"{}\", algorithm=\"SHA-256\"",
                    conf.GetWebDavRealm(), utils::generate_unique_key(), utils::generate_unique_key());
    res.add_header("WWW-Authenticate", header_content);
    res.set_status(cinatra::status_type::unauthorized);
}

inline static auto ParseAuthorizationHeader(const std::string_view& text_view)
    -> std::unordered_map<std::string, std::string>
{
    std::unordered_map<std::string, std::string> map{};
    std::string text{text_view};
    utils::string::replace(text, "Digest ", "");

    std::vector<std::string> kv_set = utils::string::split(text, ",");
    for (const std::string& kv_str : kv_set)
    {
        auto pair_opt = utils::string::split2pair(kv_str, "=");
        if (pair_opt.has_value())
        {
            map.insert(std::move(pair_opt.value()));
            continue;
        }
        std::cout << std::format("[{}:{}] Can't parse: {}", __FILE__, __LINE__, kv_str);
    }

    return map;
}

// username:realm:password
inline static std::string ComputeHA1(const std::string& username)
{
    const auto& conf = ConfigReader::GetInstance();
    return utils::sha256(std::format("{}:{}:{}", conf.GetUserAccount(), conf.GetWebDavRealm(), conf.GetUserPassword()));
}

// method:uri
inline static std::string ComputeHA2(const std::string_view& method, const std::string_view& uri)
{
    return utils::sha256(std::format("{}:{}", method, uri));
}

namespace Middleware
{

bool DigestAuthentication::before(coro_http_request& req, coro_http_response& res)
{
    try
    {
        const std::string_view& authorization = req.get_header_value("Authorization");
        if (authorization.empty())
        {
            RequestVerification(res);
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
        std::cout << std::format("[{}:{}] {}", __FILE__, __LINE__, err.what());
        res.set_status(cinatra::status_type::internal_server_error);
        return false;
    }

    return true;
}

bool DigestAuthentication::after(coro_http_request& req, coro_http_response& res)
{
    return true;
}

} // namespace Middleware
