//
// Created by lp on 2019/11/30.
//

#include "redis_client.h"
#include "commlib/logging.h"

namespace Redis {

void GetCallback(redisAsyncContext *c, void *r, void *privdata) {
  auto info = static_cast<CoYield *> (privdata);
  if (info != nullptr) {
    info->ResumeOneWithMsg(r);
  }
}

void ConnectCallback(const redisAsyncContext *c, int status) {
  if (status != REDIS_OK) {
    ERROR("Error: {}", c->errstr);
    static_cast<ClientBase*>(c->data)->SetDisConnected();
    return;
  }
  static_cast<ClientBase*>(c->data)->SetConnected();
  DEBUG("redis Connected...");
}

void DisconnectCallback(const redisAsyncContext *c, int status) {
  if (status != REDIS_OK) {
    ERROR("Error: {}", c->errstr);
    static_cast<ClientBase*>(c->data)->SetDisConnected();
    return;
  }
  static_cast<ClientBase*>(c->data)->SetDisConnected();
  DEBUG("redis Disconnected...");
}

int RedisClient::Init(std::string ip, int port) {
 ip_ = std::move(ip);
 port_ = port;
 Reconnect();
 return 0;
}

void RedisClient::Reconnect() {
  if (NotNeedReconnect()) {
    return ;
  }

  context_ = redisAsyncConnect(ip_.c_str(), port_);
  if (context_ == nullptr) {
    ERROR("context null");
    return;
  }
  if (context_->err) {
    ERROR("Error: {}", context_->errstr);
    redisAsyncFree(context_);
    return;
  }
  SetConnecting();
  context_->data = this;
  auto ret = redisLibeventAttach(context_, &base_);
//  if (ret != REDIS_OK) { return ret; }

  ret = redisAsyncSetConnectCallback(context_, ConnectCallback);
//  if (ret != REDIS_OK) { return ret; }

  ret = redisAsyncSetDisconnectCallback(context_, DisconnectCallback);
//  if (ret != REDIS_OK) { return ret; }
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
    ERROR("redis reply null. maybe need reconnect");
    throw std::logic_error("reply null");
  }

  auto redis_reply = static_cast<redisReply *>(msg);
  if (redis_reply->type == REDIS_REPLY_ERROR) {
    throw std::logic_error(std::string(redis_reply->str, redis_reply->len));
  }
  return redis_reply;
}


redisReply *RedisCmd::FormattedCmd(const std::string &cmd) {
  if (client_.State() != ClientBase::kConnected) {
    WARNING("redis not connected");
    return {};
  }
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
  if (client_.State() != ClientBase::kConnected) {
    WARNING("redis not connected");
    return {};
  }
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
