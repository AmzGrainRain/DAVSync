#include "RedisFilePropService.h"

#include <cassert>
#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>

#include "ConfigReader.h"

namespace FilePropService
{

RedisFilePropService::RedisFilePropService()
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

auto RedisFilePropService::Exec(const std::string& command) -> RedisFilePropService::ReplyT
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

bool RedisFilePropService::Set(const std::string& path_sha, const std::pair<std::string, std::string>& prop)
{
    std::string command = std::format(R"(HSET prop:{} {} "{}")", path_sha, prop.first, prop.second);
    ReplyT repl = Exec(command);

    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

std::string RedisFilePropService::Get(const std::string& path_sha, const std::string& key)
{
    std::string command = std::format("HGET prop:{} {}", path_sha, key);
    ReplyT repl = Exec(command);

    if (!repl)
    {
        return {""};
    }

    return {repl->str};
}

std::vector<PropT> RedisFilePropService::GetAll(const std::string& path_sha)
{
    std::string command = std::format("HGETALL prop:{}", path_sha);
    ReplyT repl = Exec(command);

    if (!repl)
    {
        return {};
    }

    std::vector<PropT> prop_list{};
    for (size_t i = 0; i < repl->elements; i += 2)
    {
        prop_list.push_back({(repl.get())[i].str, (repl.get())[i + 1].str});
    }

    return prop_list;
}

bool RedisFilePropService::Remove(const std::string& path_sha, const std::string& key)
{
    std::string command = std::format("HDEL prop:{} {}", path_sha, key);
    ReplyT repl = Exec(command);

    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

bool RedisFilePropService::RemoveAll(const std::string& path_sha)
{
    std::string command = std::format("DEL prop:{}", path_sha);
    ReplyT repl = Exec(command);

    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

} // namespace FilePropService
