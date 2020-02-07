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
#include "commlib/optional.h"

namespace Redis {

void GetCallback(redisAsyncContext *c, void *r, void *privdata);
void ConnectCallback(const redisAsyncContext *c, int status);
void DisconnectCallback(const redisAsyncContext *c, int status);

inline bool is_array(redisReply *reply) { return reply->type == REDIS_REPLY_ARRAY; }
inline bool is_string(redisReply * reply) { return reply->type == REDIS_REPLY_STRING; }
inline bool is_integer(redisReply * reply) { return reply->type == REDIS_REPLY_INTEGER; }
inline bool is_status(redisReply * reply) { return reply->type == REDIS_REPLY_STATUS; }
inline bool is_nil(redisReply * reply) { return reply->type == REDIS_REPLY_NIL; }

class RedisClient {
  struct event_base &base_;
  redisAsyncContext *context_ = nullptr;
 public:
  RedisClient(struct event_base &base) : base_(base){}
  redisAsyncContext *Context() { return context_; }
  int Init(std::string ip, int port);
};

class RedisCmd {
  redisAsyncContext *context_;
  CoYield &yield_;

  redisReply *Yield();

 public:

  RedisCmd(RedisClient &client, CoYield &yield)
      : context_(client.Context()),
        yield_(yield) {}
  redisReply *Cmd(const std::string &cmd);
  redisReply *Cmd(const std::vector<std::string> &cmd);
};

}

