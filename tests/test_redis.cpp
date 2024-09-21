#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>

#include <hiredis/hiredis.h>

int main()
{
    std::unique_ptr<redisContext> ctx_{redisConnect("127.0.0.1", 6379)};
    if (!ctx_)
    {
        std::cerr << "alloc err";
        return 1;
    }

    if (ctx_->err)
    {
        std::cerr << ctx_->errstr;
        return 2;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(ctx_.get(), "AUTH default asdasd"));
    std::cout << reply->str << std::endl;
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(ctx_.get(), "HGETALL test");
    std::cout << "elements:" << reply->elements << std::endl;
    for (size_t i = 0; i < reply->elements; i += 2)
    {
        std::cout << "k: " << reply->element[i]->str;
        std::cout << "\tv: " << reply->element[i + 1]->str << std::endl;
    }
    freeReplyObject(reply);

    return 0;
}
