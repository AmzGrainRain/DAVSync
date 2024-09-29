#include "RedisFileLockService.h"

#include <chrono>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <format>

#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <stdexcept>
#include <string>

#include "ConfigReader.h"
#include "logger.hpp"
#include "services/file_lock/FileLockService.h"
#include "utils/path.h"
#include "utils/redis.h"

using namespace utils::redis;

static FileLockService::FileLock ParseLock(const redisReply* const repl)
{
    FileLockService::FileLock lock{};
    for (size_t i = 0; i < repl->elements; i += 2)
    {
        if (repl->element[i]->type != REDIS_REPLY_STRING)
        {
            continue;
        }

        if (std::strcmp(repl->element[i]->str, "token") == 0)
        {
            lock.token = repl->element[i + 1]->str;
        }
        else if (std::strcmp(repl->element[i]->str, "path") == 0)
        {
            lock.path = repl->element[i + 1]->str;
        }
        else if (std::strcmp(repl->element[i]->str, "depth") == 0)
        {
            lock.depth = std::stoi(repl->element[i + 1]->str);
        }
        else if (std::strcmp(repl->element[i]->str, "scope") == 0)
        {
            lock.scope = std::stoi(repl->element[i + 1]->str);
        }
        else if (std::strcmp(repl->element[i]->str, "type") == 0)
        {
            lock.type = std::stoi(repl->element[i + 1]->str);
        }
        else if (std::strcmp(repl->element[i]->str, "expires_at") == 0)
        {
            lock.expires_at = std::stoll(repl->element[i + 1]->str);
        }
        else if (std::strcmp(repl->element[i]->str, "creation_date") == 0)
        {
            lock.creation_date = std::stoll(repl->element[i + 1]->str);
        }
    }

    return lock;
}

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

bool RedisFileLockService::Lock(const FileLock& lock) noexcept
{
    // token @ token, path, depth, scope, type, expires_at, creation_date
    const std::string command =
        std::format("HSET lock:{} token {} path {} depth {} scope {} type {} expires_at {} creation_date", lock.token, lock.token, lock.path,
                    lock.depth, static_cast<int>(lock.scope), static_cast<int>(lock.type), lock.expires_at, lock.creation_date);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->integer != 1)
    {
        if (redis_ctx_->err)
            LOG_WARN(redis_ctx_->errstr)
        return false;
    }

    token_map_.insert({lock.path, lock.token});
    return true;
}

bool RedisFileLockService::Lock(FileLock&& lock) noexcept
{
    return Lock(lock);
}

bool RedisFileLockService::Unlock(const std::string& token) noexcept
{
    std::filesystem::path path{};
    {
        std::string command = std::format("HGET lock:{} path", token);
        auto repl = RedisExecute(redis_ctx_.get(), command);
        if (!repl || repl->type == REDIS_REPLY_NIL)
        {
            if (redis_ctx_->err)
                LOG_WARN(redis_ctx_->errstr)
            return false;
        }

        path = repl->str;
    }

    std::string command = std::format("DEL lock:{}", token);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->type != REDIS_REPLY_INTEGER || repl->integer != 1)
    {
        if (redis_ctx_->err)
            LOG_WARN(redis_ctx_->errstr)
        return false;
    }

    token_map_.erase(path);
    return true;
}

bool RedisFileLockService::IsLocked(const std::string& token) noexcept
{
    using namespace std::chrono;

    const std::string command = std::format("HGET lock:{} expire", token);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->type == REDIS_REPLY_NIL)
    {
        if (redis_ctx_->err)
            LOG_WARN(redis_ctx_->errstr)
        return false;
    }

    seconds expire_sec{std::stoull(std::string{repl->str})};
    auto now = system_clock::now().time_since_epoch();
    auto now_sec = duration_cast<seconds>(now);
    return expire_sec > now_sec;
}

bool RedisFileLockService::IsLocked(const std::filesystem::path& path) noexcept
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
        if (redis_ctx_->err)
            LOG_WARN(redis_ctx_->errstr)
        return false;
    }

    seconds expire_sec{std::stoull(std::string{repl->str})};
    auto now = system_clock::now().time_since_epoch();
    auto now_sec = duration_cast<seconds>(now);
    return expire_sec > now_sec;
}

FileLock RedisFileLockService::GetLock(const std::string& token) noexcept(false)
{
    const std::string command = std::format("HGETALL lock:{}", token);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || redis_ctx_->err || repl->type != REDIS_REPLY_ARRAY || repl->elements % 2)
    {
        throw std::runtime_error(redis_ctx_->errstr);
    }

    return ParseLock(repl.get());
}

FileLock RedisFileLockService::GetLock(const std::filesystem::path& path) noexcept(false)
{
    return GetLock(utils::path::to_string(path));
}

} // namespace FileLockService
