#include "RedisFileLockService.h"

#include <chrono>
#include <filesystem>
#include <format>

#include <hiredis/hiredis.h>
#include <hiredis/read.h>

#include "ConfigReader.h"
#include "utils/redis.h"

using namespace utils::redis;

namespace FileLockService
{

RedisFileLockService::RedisFileLockService() : redis_ctx_(nullptr, &redisFree)
{
    const auto& conf = ConfigReader::GetInstance();

    redis_ctx_ = GetRedisContext(conf.GetRedisHost(), conf.GetRedisPort());
    if (redis_ctx_->err)
    {
        throw std::runtime_error(redis_ctx_->errstr);
    }

    if (!RedisAuth(redis_ctx_.get(), conf.GetRedisUserName(), conf.GetRedisPassword()))
    {
        throw std::runtime_error("Auth failed.");
    }
}

bool RedisFileLockService::Lock(const FileLock& lock)
{
    // token @ token, path, depth, scope, type, expires_at, creation_date
    const std::string command =
        std::format("HSET lock:{} token {} path {} depth {} scope {} type {} expires_at {} creation_date", lock.token,
                    lock.token, lock.path, lock.depth, static_cast<int>(lock.scope), static_cast<int>(lock.type),
                    lock.expires_at, lock.creation_date);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->integer != 1)
    {
        return false;
    }

    token_map_.insert({lock.path, lock.token});
    return true;
}

bool RedisFileLockService::Lock(FileLock&& lock)
{
    return Lock(lock);
}

bool RedisFileLockService::Unlock(const std::string& token)
{
    std::filesystem::path path{};
    {
        std::string command = std::format("HGET lock:{} path", token);
        auto repl = RedisExecute(redis_ctx_.get(), command);
        if (!repl || repl->type == REDIS_REPLY_NIL)
        {
            return false;
        }

        path = repl->str;
    }

    std::string command = std::format("DEL lock:{}", token);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->type != REDIS_REPLY_INTEGER || repl->integer != 1)
    {
        return false;
    }

    token_map_.erase(path);
    return true;
}

bool RedisFileLockService::IsLocked(const std::string& token)
{
    using namespace std::chrono;

    const std::string command = std::format("HGET lock:{} expire", token);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->type == REDIS_REPLY_NIL)
    {
        return false;
    }

    seconds expire_sec{std::stoull(std::string{repl->str})};
    auto now = system_clock::now().time_since_epoch();
    auto now_sec = duration_cast<seconds>(now);
    return expire_sec > now_sec;
}

bool RedisFileLockService::IsLocked(const std::filesystem::path& path)
{
    using namespace std::chrono;

    const auto& token_it = token_map_.find(path);
    if (token_it == token_map_.end())
    {
        return false;
    }

    const std::string command = std::format("HGET lock:{} expires_at", token_it->second);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->type == REDIS_REPLY_NIL)
    {
        return false;
    }

    seconds expire_sec{std::stoull(std::string{repl->str})};
    auto now = system_clock::now().time_since_epoch();
    auto now_sec = duration_cast<seconds>(now);
    return expire_sec > now_sec;
}

} // namespace FileLockService
