#include "digest_authentication.h"

#include "../utils.h"

namespace Middleware
{

bool DigestAuthentication::before(coro_http_request& req, coro_http_response& res)
{


    std::string nonce = generate_unique_key();
    std::string opaque = "opaque_" + generate_unique_key();
    res.add_header("WWW-Authenticate",
                   "Digest realm=all, nonce=" + nonce + ", opaque=" + opaque + ", algorithm=SHA-256");
    res.set_status(cinatra::status_type::unauthorized);

    return false;
}

bool DigestAuthentication::after(coro_http_request& req, coro_http_response& res)
{
    return true;
}

} // namespace Middleware
