#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <memory>

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include "commlib/redis_client.h"
#include "commlib/assert.hpp"

void Debug(const std::vector<std::string> &value) {
  for (auto &item : value) {
    std::cout << item << std::endl;
  }
}

using namespace Redis;
int main(int argc, char **argv) {
  struct event_base *base = event_base_new();
  CoTask co_task_;
  RedisClient redis_client(*base, co_task_);
  redis_client.Init("127.0.0.1", 6379);

  co_task_.DoTack([&redis_client](const CoYield &yield) {
    try {
      RedisCmd cmd(redis_client, yield);
      cmd.Set("key1", "value1");
      cmd.Set("key2", "value2");
      char b[6] = {0, 1, 2, 122, 9};
      cmd.Set("key3", std::string(b, sizeof(b)));
      std::cout << "value1:" << cmd.Get("key1") << std::endl;
      std::cout << "value2:" << cmd.Get("key2") << std::endl;
      std::cout << "value3:" << cmd.Get("key3").size() << std::endl;

      Debug(cmd.MGet({"key1", "key2"}));
      cmd.Set("incr_key", "0");
      cmd.Incr("incr_key");
      cmd.Incr("incr_key");
      cmd.Incr("incr_key");
      ASSERT_EQ(cmd.Get<int>("incr_key"), 3);
      redisAsyncDisconnect(redis_client.Context());
    } catch (const std::exception &e) {
      std::cout << "redis cmd error: " << e.what();
    }
  });

  event_base_dispatch(base);
  return 0;
}
