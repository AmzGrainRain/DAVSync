#include "RedisFileLockService.h"

#include <chrono>
#include <format>

#include <hiredis/hiredis.h>
#include <hiredis/read.h>

#include "ConfigReader.h"
#include "utils/path.h"
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

bool RedisFileLockService::Lock(const std::filesystem::path& path, FileLockType type, std::chrono::seconds expire_time)
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command =
        std::format("HSET lock:{} type {} expire {}", path_str, static_cast<int>(type), expire_time.count());

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

bool RedisFileLockService::Unlock(const std::filesystem::path& path)
{
    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("DEL lock:{}", path_str);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    return repl->integer == 1;
}

bool RedisFileLockService::IsLocked(const std::filesystem::path& path)
{
    using namespace std::chrono;

    const std::string path_str = utils::path::to_string(path);
    const std::string command = std::format("HGET lock:{} expire", path_str);

    RedisReplyT repl = RedisExecute(redis_ctx_.get(), command);
    if (!repl)
    {
        return false;
    }

    seconds expire_sec{std::stoull(std::string{repl->str})};
    auto now = system_clock::now().time_since_epoch();
    auto now_sec = duration_cast<seconds>(now);
    return expire_sec > now_sec;
}

} // namespace FileLockService
