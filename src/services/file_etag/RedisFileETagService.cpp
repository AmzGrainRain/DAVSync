#include "RedisFileETagService.h"

#include <cassert>
#include <format>
#include <iostream>

#include <hiredis/hiredis.h>

#include "ConfigManager.h"
#include "logger.hpp"
#include "utils.h"
#include "utils/path.h"
#include "utils/redis.h"

using namespace utils::redis;

namespace FileETagService
{

RedisFileETagService::RedisFileETagService() : redis_ctx_(nullptr, &redisFree)
{
    const auto& conf = ConfigManager::GetInstance();

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

std::string RedisFileETagService::Get(const std::filesystem::path& path) noexcept
{
    const std::string key = utils::path::to_string(path);
    const std::string command = std::format("GET etag:{}", key);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return {""};
    }

    return {repl->str};
}

std::string RedisFileETagService::Set(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    std::string sha{};
    if (std::filesystem::is_directory(path))
    {
        sha = utils::sha256(path_str);
    }
    else if (std::filesystem::is_regular_file(path))
    {
        sha = utils::sha256(path);
    }
    else
    {
        LOG_WARN("Unexpected file type.")
        return {""};
    }

    const std::string command = std::format(R"(SET etag:{} "{}")", path_str, sha);
    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl || repl->integer != 1)
    {
        return {""};
    }

    return sha;
}

} // namespace FileETagService
