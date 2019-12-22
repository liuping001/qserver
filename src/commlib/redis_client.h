//
// Created by lp on 2019/11/30.
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <memory>

#include "commlib/co/co_task.h"
#include "commlib/type_help.hpp"

#include "third/hiredis-vip/hiredis.h"
#include "third/hiredis-vip/async.h"
#include "third/hiredis-vip/adapters/libevent.h"

#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include "optional.h"

namespace Redis {

void GetCallback(redisAsyncContext *c, void *r, void *privdata);
void ConnectCallback(const redisAsyncContext *c, int status);
void DisconnectCallback(const redisAsyncContext *c, int status);

class RedisClient {
  struct event_base &base_;
  CoTask &co_task_;
  redisAsyncContext *context_ = nullptr;
 public:
  RedisClient(struct event_base &base, CoTask &co_task) : base_(base), co_task_(co_task) {}
  redisAsyncContext *Context() { return context_; }
  CoTask &GetCoTask() { return co_task_; }
  int Init(std::string ip, int port);
};

class RedisCmd {
  CoTask &co_task_;
  redisAsyncContext *context_;
  const CoYield &yield_;

  redisReply *Yield();

  template<class T, class ...Args>
  optional<T> InnerCmd(Args &&...args);
 public:

  RedisCmd(RedisClient &client, const CoYield &yield)
      : co_task_(client.GetCoTask()),
        context_(client.Context()),
        yield_(yield) {}

  void set(const std::string &key, const std::string &value);
  bool setnx(const std::string &key, const std::string &value);
  void setex(const std::string &key, int64_t seconds, const std::string &value);
  std::vector<optional<std::string>> mget(const std::vector<std::string> &keys);
  optional<long long> incr(const std::string &key);
  optional<std::string> get(const std::string &key);
  template<class T>
  optional<T> get(const std::string &key) {
    auto value = get(key);
    if (value) {
      type::string_to<T>(value.value());
    }
    return value;
  }
};

}

