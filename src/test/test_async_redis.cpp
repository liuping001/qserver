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

void Debug(const std::vector<optional<std::string>> &value) {
  for (auto &item : value) {
    std::cout << item.value() << std::endl;
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
      cmd.set("key1", "value1");
      cmd.set("key2", "value2");
      char b[6] = {0, 1, 2, 122, 9};
      cmd.set("key3", std::string(b, sizeof(b)));
      std::cout << "value1:" << cmd.get("key1").value() << std::endl;
      std::cout << "value2:" << cmd.get("key2").value() << std::endl;
      std::cout << "value3:" << cmd.get("key3").value().size() << std::endl;

      Debug(cmd.mget({"key1", "key2"}));
      cmd.set("incr_key", "0");
      cmd.incr("incr_key");
      cmd.incr("incr_key");
      cmd.incr("incr_key");
      ASSERT_EQ(cmd.get<int>("incr_key").value(), 3);
      redisAsyncDisconnect(redis_client.Context());
    } catch (const std::exception &e) {
      std::cout << "redis cmd error: " << e.what();
    }
  });

  event_base_dispatch(base);
  return 0;
}
