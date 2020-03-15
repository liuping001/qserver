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

struct Status {
  enum STATUS {
    NONE,
    OK,
  };
  bool Ok() { return status == OK; }
  void SetOk() { status = OK; }
  STATUS status = OK;
};

bool is_array(redisReply *reply) { return reply->type == REDIS_REPLY_ARRAY; }
bool is_string(redisReply * reply) { return reply->type == REDIS_REPLY_STRING; }
bool is_integer(redisReply * reply) { return reply->type == REDIS_REPLY_INTEGER; }
bool is_status(redisReply * reply) { return reply->type == REDIS_REPLY_STATUS; }
bool is_nil(redisReply * reply) { return reply->type == REDIS_REPLY_NIL; }

class RedisReplyReader {
 private:
  redisReply *reply_;
 public:
  RedisReplyReader(redisReply *reply) : reply_(reply) {}
  void ReadValue(std::string &value) {
    if (!is_string(reply_)) {
      throw RedisEx("not string type");
    }
    value.assign(reply_->str, reply_->len);
  }
  void ReadValue(long long &value) {
    if (!is_integer(reply_)) {
      throw RedisEx("not integer type");
    }
    value = reply_->integer;
  }
  void ReadValue(bool &value) {
    if (!is_integer(reply_)) {
      throw RedisEx("not integer type");
    }
    value = reply_->integer;
  }

  void ReadValue(Status &value) {
    if (!is_status(reply_)) {
      throw RedisEx("not status type");
    }
    std::string str(reply_->str, reply_->len);
    if (str == "OK") {
      value.SetOk();
    }
  }

  template<class T>
  void ReadValue(std::vector<T> &value) {
    if (!is_array(reply_)) {
      throw RedisEx("not array type");
    }
    for (size_t i = 0; i < reply_->elements; i++) {
      value.emplace_back();
      RedisReplyReader reader(reply_->element[i]);
      reader.ReadValue(value.back().value());
    }
  }
};

namespace writer {
template<class T>
void Append(std::vector<std::string> &cmd, T &&t) {
  using namespace std;
  cmd.push_back(to_string(std::forward<T>(t)));
}

template<class T>
void Append(std::vector<std::string> &cmd, const std::vector<T> &t) {
  for (const auto &item : t) {
    Append(cmd, item);
  }
}

template<class H, class ...T>
void Append(std::vector<std::string> &cmd, H &&h, T &&... t) {
  using namespace std;
  cmd.push_back(to_string(std::forward<H>(h)));
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
optional<T> RedisCmd::InnerCmd(Args &&...args) {
  auto cmd = writer::Cmd(std::forward<Args>(args)...);
  std::vector<const char *> argvs(cmd.size());
  std::vector<size_t> argvlens(cmd.size());
  for (uint32_t i = 0; i < cmd.size(); ++i) {
    argvs[i] = cmd[i].c_str();
    argvlens[i] = cmd[i].length();
  }

  redisAsyncCommandArgv(context_, GetCallback, (void *)&yield_, argvs.size(), &argvs[0], &argvlens[0]);
  auto reply = Yield();
  if (is_nil(reply)) {
    return {};
  }
  RedisReplyReader reader(reply);
  optional<T> t;
  reader.ReadValue(t.value());
  return t;
}


void RedisCmd::set(const std::string &key, const std::string &value) {
  InnerCmd<Status>("set", key, value);
  return;
}

bool RedisCmd::setnx(const std::string &key, const std::string &value) {
  return InnerCmd<bool>("setnx ", key, value).value();
}

void RedisCmd::setex(const std::string &key, int64_t seconds, const std::string &value) {
  InnerCmd<Status> ("setex ", key, seconds, value);
  return;
}

optional<std::string> RedisCmd::get(const std::string &key) {
  return InnerCmd<std::string>("get", key);
}

std::vector<optional<std::string>> RedisCmd::mget(const std::vector<std::string> &keys) {
  return InnerCmd<std::vector<optional<std::string>>>("mget", keys).value();
}

optional<long long> RedisCmd::incr(const std::string &key) {
  return InnerCmd<long long>("incr", key);
}
} // end namespace
