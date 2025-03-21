#include "redis.h"

#include <hiredis/hiredis.h>

#include <format>
#include <stdexcept>

namespace utils::redis
{

RedisContextT GetRedisContext(const std::string& host, int port) noexcept(false)
{
    redisContext* ptr = redisConnect(host.data(), port);
    if (!ptr || ptr->err)
    {
        throw std::runtime_error("Redis connection error.");
    }

    return {ptr, &redisFree};
}

auto RedisExecute(redisContext* ctx, const std::string& command) noexcept -> RedisReplyT
{
    RedisReplyT repl{static_cast<redisReply*>(redisCommand(ctx, command.data())), &freeReplyObject};
    if (ctx->err)
    {
        repl.reset();
    }

    if (!repl)
    {
        repl.reset();
    }

    if (repl->type == REDIS_REPLY_NIL)
    {
        repl.reset();
    }

    return std::move(repl);
}

bool RedisAuth(redisContext* ctx, const std::string& user, const std::string& password) noexcept
{
    const std::string auth_str = std::format("AUTH {} {}", user, password);
    const RedisReplyT repl = RedisExecute(ctx, auth_str);
    if (!repl)
    {
        return false;
    }

    return std::strcmp(repl->str, "OK") == 0;
}

} // namespace utils::redis
