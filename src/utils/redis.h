#pragma once

#include <memory>
#include <string>

#include <hiredis/hiredis.h>

namespace utils::redis
{

using RedisContextT = std::unique_ptr<redisContext, decltype(&redisFree)>;
using RedisReplyT = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

[[nodiscard]]
RedisContextT GetRedisContext(const std::string& host, int port) noexcept(false);

[[nodiscard]]
RedisReplyT RedisExecute(redisContext* ctx, const std::string& command) noexcept;

bool RedisAuth(redisContext* ctx, const std::string& user, const std::string& password) noexcept;

} // namespace utils::redis
