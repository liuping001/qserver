//
// Created by liuping on 2020/2/5.
//


#include "commlib/redis/connection.h"

class SvrCommTrans;
namespace sw {

namespace redis {

class RedisConnect : public Connection {
 public:
  RedisConnect(SvrCommTrans &trans);

 private:
  SvrCommTrans &_trans;
  void *_reply;

  ReplyUPtr recv() final;

  void redisAppendCommandArgv(int argc, const char **argv, const size_t *argvlen) final ;

};

} // end namespace redis

} // end namespace sw

