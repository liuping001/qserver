//
// Created by lp on 2019/11/30.
//

#include "redis_client.h"

namespace Redis {

void GetCallback(redisAsyncContext *c, void *r, void *privdata) {
  auto info = static_cast<CoYield *> (privdata);
  if (info != nullptr) {
    info->ResumeOneWithMsg(r);
  }
}

void ConnectCallback(const redisAsyncContext *c, int status) {
  if (status != REDIS_OK) {
    printf("Error: %s\n", c->errstr);
    return;
  }
  printf("Connected...\n");
}

void DisconnectCallback(const redisAsyncContext *c, int status) {
  if (status != REDIS_OK) {
    printf("Error: %s\n", c->errstr);
    return;
  }
  printf("Disconnected...\n");
}

int RedisClient::Init(std::string ip, int port) {
  context_ = redisAsyncConnect(ip.c_str(), port);
  if (context_->err) {
    printf("Error: %s\n", context_->errstr);
    return -1;
  }
  auto ret = redisLibeventAttach(context_, &base_);
  if (ret != REDIS_OK) { return ret; }

  ret = redisAsyncSetConnectCallback(context_, ConnectCallback);
  if (ret != REDIS_OK) { return ret; }

  ret = redisAsyncSetDisconnectCallback(context_, DisconnectCallback);
  if (ret != REDIS_OK) { return ret; }
  return 0;
}

class RedisEx : public std::exception {
 public:
  RedisEx(const std::string &what) : what_(what) { }
  const char* what() const noexcept final { return what_.c_str(); }
 private:
  std::string what_;
};


redisReply *RedisCmd::Yield() {
  auto ret = yield_.Yield();
  if (ret != 0) {
    throw std::logic_error("yield failed");
  }
  auto msg = yield_.GetMsg();
  if (msg == nullptr) {
    throw std::logic_error("reply null");
  }

  auto redis_reply = static_cast<redisReply *>(msg);
  if (redis_reply->type == REDIS_REPLY_ERROR) {
    throw std::logic_error(std::string(redis_reply->str, redis_reply->len));
  }
  return redis_reply;
}


redisReply *RedisCmd::Cmd(const std::string &cmd) {
  auto ret = redisAsyncFormattedCommand(context_, GetCallback, (void *)&yield_, cmd.data(), cmd.size());
  if (ret != 0) {
    return {};
  }
  auto reply = Yield();
  if (is_nil(reply)) {
    return {};
  }
  return reply;
}

redisReply * RedisCmd::Cmd(const std::vector<std::string> & cmd_list) {
  std::vector<size_t> argc;
  std::vector<const char *> argv;
  for (auto & item : cmd_list) {
    argc.push_back(item.size());
    argv.push_back(item.data());
  }
  auto ret = redisAsyncCommandArgv(context_, GetCallback, (void *)&yield_, (int)cmd_list.size(), argv.data(), argc.data());
  if (ret != 0) {
    return {};
  }
  auto reply = Yield();
  if (is_nil(reply)) {
    return {};
  }
  return reply;
}

} // end namespace
