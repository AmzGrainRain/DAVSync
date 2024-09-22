#include "RedisFileETagService.h"

#include <cassert>

#include <cstring>
#include <format>
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>

#include "ConfigReader.h"
#include "utils.h"
#include "utils/path.h"

namespace FileETagService
{

RedisFileETagService::RedisFileETagService()
{
    const auto& conf = ConfigReader::GetInstance();

    redis_ctx_ = std::unique_ptr<redisContext>(redisConnect(conf.GetRedisHost().data(), conf.GetRedisPort()));
    assert(redis_ctx_);
    if (redis_ctx_->err)
    {
        throw std::runtime_error(redis_ctx_->errstr);
    }

    auth_str_ = std::format("AUTH {} {}", conf.GetRedisUserName(), conf.GetRedisPassword()).data();
    ReplyT repl{static_cast<redisReply*>(redisCommand(redis_ctx_.get(), auth_str_.data())), &freeReplyObject};
    assert(repl);
    if (std::strcmp(repl->str, "OK") != 0)
    {
        throw std::runtime_error("Auth failed.");
    }
}

auto RedisFileETagService::Exec(const std::string& command) -> RedisFileETagService::ReplyT
{
    ReplyT repl{static_cast<redisReply*>(redisCommand(redis_ctx_.get(), command.data())), &freeReplyObject};
    if (!repl)
    {
        spdlog::error("Memory allocation error occurred.");
        throw std::runtime_error("Memory allocation error occurred.");
    }

    if (redis_ctx_->err)
    {
        spdlog::warn(redis_ctx_->errstr);
        repl.reset();
    }

    return std::move(repl);
}

std::string RedisFileETagService::Get(const std::filesystem::path& path)
{
    const std::string key = utils::path::to_string(path);
    const std::string command = std::format("GET etag:{}", key);

    ReplyT repl = Exec(command);
    if (!repl)
    {
        return {""};
    }

    return {repl->str};
}

bool RedisFileETagService::Set(const std::filesystem::path& path)
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
        throw std::runtime_error("Unexpected file type.");
    }

    const std::string command = std::format(R"(SET etag:{} "{}")", path_str, sha);
    ReplyT repl = Exec(command);
    if (!repl)
    {
        return false;
    }

    return true;
}

} // namespace FileETagService
