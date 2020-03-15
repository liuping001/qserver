//
// Created by liuping on 2020/2/5.
//


#include "commlib/redis/connection.h"
// redis命令执行器: 发送命令，并等待结果
// 基于Trans收发消息
class SvrCommTrans;
namespace sw {

namespace redis {

class RedisActuator : public Connection {
 public:
  RedisActuator(SvrCommTrans &trans);

 private:
  SvrCommTrans &_trans;
  void *_reply;

  ReplyUPtr recv() final;

  void redisCommandArgv(int argc, const char **argv, const size_t *argvlen) final ;

  void redisCommandFormatted(std::string &&cmd);
};

} // end namespace redis

} // end namespace sw

