//
// Created by mico on 2019/12/17.
//

#include <iostream>
#include "redis++/redis++.h"

class Redis {
 public:
  void Init(const std::string& ip, int port);
  static Redis& Inst();
#ifdef REDIS_CLUSTER_MODE
  std::shared_ptr<sw::redis::RedisCluster> redis;
#else
  std::shared_ptr<sw::redis::Redis> redis;
#endif
 private:
  bool inited_ = false;
};

void Redis::Init(const std::string &ip, int port) {
  inited_ = true;
  sw::redis::ConnectionOptions redis_connection_opts;
  redis_connection_opts.host = ip;
  redis_connection_opts.port = port;
  //redis_connection_opts.password = "";
  redis_connection_opts.connect_timeout = std::chrono::milliseconds(500);
  //redis_connection_opts.socket_timeout = std::chrono::milliseconds(10000);

#ifdef REDIS_CLUSTER_MODE
  redis = std::make_shared<sw::redis::RedisCluster>(redis_connection_opts);
#else
  redis = std::make_shared<sw::redis::Redis>(redis_connection_opts);
#endif
}

Redis &Redis::Inst() {
  thread_local static Redis client;
  if (!client.inited_) {
    client.Init("127.0.0.1", 6379);
  }
  return client;
}

#include <string>
#include <sstream>
#include <vector>

std::vector<std::string> Split(const std::string &s, char delim = ' ') {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    if (!item.empty()) {
      elems.push_back(std::move(item));
    }
  }
  return elems;
}

int main() {
  try {
    Redis::Inst().Init("127.0.0.1", 6379);
    auto redis = Redis::Inst().redis;
    redis->command("set","key11", "value11"); // redis->command("set key11 value11") invalid
    redis->command("incr","key13");

    auto cmds = Split("set key16 091");
    redis->command(cmds.begin(), cmds.end());
  } catch (const sw::redis::Error &err) {
    std::cout << err.what();
  }
  return 0;
}