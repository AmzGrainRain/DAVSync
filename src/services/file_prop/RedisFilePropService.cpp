#include "RedisFilePropService.h"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <hiredis/hiredis.h>

#include "ConfigManager.h"
#include "utils/path.h"

using namespace utils::redis;

namespace FilePropService
{

RedisFilePropService::RedisFilePropService() noexcept(false) : redis_ctx_(nullptr, &redisFree)
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

bool RedisFilePropService::Set(const std::filesystem::path& path, const std::pair<std::string, std::string>& prop) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format(R"(HSET prop:{} {} "{}")", path_str, prop.first, prop.second);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

std::string RedisFilePropService::Get(const std::filesystem::path& path, const std::string& key) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("HGET prop:{} {}", path_str, key);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return {""};
    }

    return {repl->str};
}

std::vector<PropT> RedisFilePropService::GetAll(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("HGETALL prop:{}", path_str);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
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

bool RedisFilePropService::Remove(const std::filesystem::path& path, const std::string& key) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("HDEL prop:{} {}", path_str, key);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

bool RedisFilePropService::RemoveAll(const std::filesystem::path& path) noexcept
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("DEL prop:{}", path_str);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

} // namespace FilePropService
