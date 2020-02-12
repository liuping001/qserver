//
// Created by liuping on 2020/1/26.
//

#pragma once
#include "reply.h"
#include <cstdarg>
#include <vector>

namespace sw {

namespace redis {

class CmdArgs;

class Connection {
 public:
  Connection() {};
  virtual ~Connection() {}

  virtual ReplyUPtr recv() { return nullptr; }

  virtual void redisCommandArgv(int argc, const char **argv, const size_t *argvlen) {}
  virtual void redisCommandFormatted(std::string &&cmd) {}
  template<typename ...Args>
  void send(const char *format, Args &&...args);

  void send(int argc, const char **argv, const std::size_t *argv_len) ;

  void send(CmdArgs &args);

 private:
  std::string redisAppendCommand(const char *format, ...) {
    char * cmd = nullptr;
    va_list ap;
    va_start(ap,format);
    auto len = redisvFormatCommand(&cmd, format,ap);
    va_end(ap);
    if (len < 0) {
      return {};
    }
    std::string ret(cmd, len);
    free(cmd);
    return ret;
  }

  int redisvFormatCommand(char **target, const char *format, va_list ap);
};

template <typename ...Args>
inline void Connection::send(const char *format, Args &&...args) {
  auto cmd = redisAppendCommand(format, std::forward<Args>(args)...);
  if (cmd.empty()) {
    return;
  }
  redisCommandFormatted(std::move(cmd));
}

}
}
