#pragma once

#include <memory>
#include <string>

#include <hiredis/hiredis.h>

namespace utils::redis
{

using RedisContextT = std::unique_ptr<redisContext, decltype(&redisFree)>;
using RedisReplyT = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

RedisContextT GetRedisContext(const std::string& host, int port);

RedisReplyT RedisExecute(redisContext* ctx, const std::string& command);

bool RedisAuth(redisContext* ctx, const std::string& user, const std::string& password);

} // namespace utils::redis
