#pragma once

class RedisClient : public DataClient
{
  public:
    RedisClient();



  private:


    redisOptions redis_options_;
};
