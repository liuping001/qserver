//
// Created by lp on 2019/11/30.
//

#include "redis_client.h"

namespace Redis {
struct CoInfo {
  CoTask &co_task;
  const CoYield &yield;
};

void GetCallback(redisAsyncContext *c, void *r, void *privdata) {
  auto info = static_cast<CoInfo *> (privdata);
  if (info != nullptr) {
    info->co_task.ResumeOneWithMsg(info->yield.co_id_, r);
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

class RedisReplyReader {
 private:
  redisReply *reply_;
 public:
  RedisReplyReader(redisReply *reply) : reply_(reply) {}
  void ReadValue(std::string &value) { value.assign(reply_->str, reply_->len); }
  void ReadValue(long long &value) { value = reply_->integer; }

  template<class T>
  void ReadValue(std::vector<T> &value) {
    if (reply_->type != REDIS_REPLY_ARRAY) {
      return;
    }
    for (size_t i = 0; i < reply_->elements; i++) {
      value.emplace_back();
      RedisReplyReader reader(reply_->element[i]);
      reader.ReadValue(value.back());
    }
  }
};

namespace writer {
template<class T>
void Append(std::vector<std::string> &cmd, T &&t) {
  cmd.push_back(std::forward<T>(t));
}

template<class T>
void Append(std::vector<std::string> &cmd, const std::vector<T> &t) {
  for (const auto &item : t) {
    Append(cmd, item);
  }
}

template<class H, class ...T>
void Append(std::vector<std::string> &cmd, H &&h, T &&... t) {
  cmd.push_back(std::forward<H>(h));
  Append(cmd, std::forward<T>(t)...);
}
template<class ...T>
std::vector<std::string> Cmd(T &&...t) {
  std::vector<std::string> cmd;
  Append(cmd, std::forward<T>(t)...);
  return cmd;
}
}

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

template<class T, class ...Args>
T RedisCmd::InnerCmd(Args &&...args) {
  auto cmd = writer::Cmd(std::forward<Args>(args)...);
  std::vector<const char *> argvs(cmd.size());
  std::vector<size_t> argvlens(cmd.size());
  for (uint32_t i = 0; i < cmd.size(); ++i) {
    argvs[i] = cmd[i].c_str();
    argvlens[i] = cmd[i].length();
  }
  CoInfo co_info{co_task_, yield_};
  redisAsyncCommandArgv(context_, GetCallback, &co_info, argvs.size(), &argvs[0], &argvlens[0]);
  auto reply = Yield();
  RedisReplyReader reader(reply);
  T t;
  reader.ReadValue(t);
  return t;
}
template<class ...Args>
void RedisCmd::NoRetInnerCmd(Args &&...args) {
  auto cmd = writer::Cmd(std::forward<Args>(args)...);
  std::vector<const char *> argvs(cmd.size());
  std::vector<size_t> argvlens(cmd.size());
  for (uint32_t i = 0; i < cmd.size(); ++i) {
    argvs[i] = cmd[i].c_str();
    argvlens[i] = cmd[i].length();
  }
  CoInfo co_info{co_task_, yield_};
  redisAsyncCommandArgv(context_, GetCallback, &co_info, argvs.size(), &argvs[0], &argvlens[0]);
  Yield();
}

void RedisCmd::Set(const std::string &key, const std::string &value) {
  return NoRetInnerCmd("set", key, value);
}

std::string RedisCmd::Get(const std::string &key) {
  return InnerCmd<std::string>("get", key);
}

std::vector<std::string> RedisCmd::MGet(const std::vector<std::string> &keys) {
  return InnerCmd<std::vector<std::string>>("mget", keys);
}

long long RedisCmd::Incr(const std::string &key) {
  return InnerCmd<long long>("incr", key);
}
} // end namespace
