#include "FileLockServiceWithRedis.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>

#include <hiredis/async.h>
#include <hiredis/hiredis.h>

#include "ConfigReader.h"
#include "FileLockService.h"

FileLockServiceWithRedis::FileLockServiceWithRedis()
{
    const auto& conf = ConfigReader::GetInstance();
    redis_options_ = {0};
    REDIS_OPTIONS_SET_TCP(&redis_options_, conf.GetRedisHost().data(), conf.GetRedisPort());
}

bool FileLockServiceWithRedis::LockFile(const std::string& lock_token, FileLockType lock_type,
                                        std::chrono::seconds lock_expire_time) const
{
    RedisContext ctx{redisConnectWithOptions(&redis_options_), redisFree};
    if (!ctx)
        throw std::runtime_error("redisContext == nullptr");
    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    switch (lock_type)
    {
    case FileLockType::NONE:
        return false;
    case FileLockType::SHARED:
        redisCommand(ctx.get(), "SET lock:%s %s", lock_token.c_str(), lock_expire_time);
        break;
    case FileLockType::EXCLUSIVE:
        redisCommand(ctx.get(), "SET lock:%s %s", lock_token.c_str(), lock_expire_time);
        break;
    }

    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    return true;
}

bool FileLockServiceWithRedis::UnlockFile(const std::string& lock_token) const
{
    redisContext* ctx = redisConnectWithOptions(&redis_options_);
    if (!ctx)
        throw std::runtime_error("redisContext == nullptr");
    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    RedisReply reply{reinterpret_cast<redisReply*>(redisCommand(ctx, "DEL %s", lock_token.data())), freeReplyObject};
    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    return true;
}

bool FileLockServiceWithRedis::IsLocked(const std::string& lock_token) const
{
    redisContext* ctx = redisConnectWithOptions(&redis_options_);
    if (!ctx)
        throw std::runtime_error("redisContext is nullptr.");
    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    RedisReply reply{reinterpret_cast<redisReply*>(redisCommand(ctx, "GET %s", lock_token.c_str())), freeReplyObject};
    if (ctx->err)
        throw std::runtime_error(ctx->errstr);

    if (reply->integer == 0)
        return false;

    return std::strcmp(reply->str, "SHARED") == 0 || std::strcmp(reply->str, "EXCLUSIVE") == 0;
}
