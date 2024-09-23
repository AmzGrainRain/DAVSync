#include "redis.h"

#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>

namespace utils::redis
{

RedisContextT GetRedisContext(const std::string& host, int port)
{
    return {redisConnect(host.data(), port), &redisFree};
}

auto RedisExecute(redisContext* ctx, const std::string& command) -> RedisReplyT
{
    RedisReplyT repl{static_cast<redisReply*>(redisCommand(ctx, command.data())), &freeReplyObject};
    if (!repl)
    {
        spdlog::error("Memory allocation error occurred.");
        throw std::runtime_error("Memory allocation error occurred.");
    }

    if (ctx->err)
    {
        spdlog::error(ctx->errstr);
        repl.reset();
    }

    if (repl->type == REDIS_REPLY_NIL)
    {
        repl.reset();
    }

    return std::move(repl);
}

bool RedisAuth(redisContext* ctx, const std::string& user, const std::string& password)
{
    const std::string auth_str = std::format("AUTH {} {}", user, password);
    RedisReplyT repl = RedisExecute(ctx, auth_str.data());
    if (!repl)
    {
        return false;
    }

    return std::strcmp(repl->str, "OK") == 0;
}

} // namespace utils::redis
