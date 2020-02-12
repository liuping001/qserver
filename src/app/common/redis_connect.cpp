//
// Created by liuping on 2020/2/5.
//

#include "redis_connect.h"
#include "commlib/co/co_task.h"
#include "svr_comm_trans.h"
#include "app/proto/cmd.pb.h"
#include "app/proto/redis_cmd.pb.h"
#include "convert_reply_help.h"

namespace sw {

namespace redis {

RedisConnect::RedisConnect(SvrCommTrans &trans) : Connection(), _trans(trans) {}

ReplyUPtr RedisConnect::recv() {
  ReplyUPtr ret((redisReply *)_reply);
  _reply = nullptr;
  return ret;
}

void RedisConnect::redisCommandArgv(int argc, const char **argv, const size_t *argvlen) {
  proto::redis::RedisCmdReq req;
  for (int i = 0; i < argc; i++) {
    req.add_cmd_argv(argv[i], argvlen[i]);
  }
  auto rsp = _trans.SendMsgRpcByType<proto::redis::RedisCmdRsp>(1, proto::cmd::kREDIS_CMD_REQ, req, 0);
  std::cout << "redis rsp:"<<rsp.ShortDebugString() <<"\n";
  _reply = ConvertReply::PbToReply(rsp.reply());
}

void RedisConnect::redisCommandFormatted(std::string &&cmd) {
  proto::redis::RedisCmdReq req;
  req.set_formatted_cmd(std::move(cmd));
  auto rsp = _trans.SendMsgRpcByType<proto::redis::RedisCmdRsp>(1, proto::cmd::kREDIS_CMD_REQ, req, 0);
  std::cout << "redis rsp:"<<rsp.ShortDebugString() <<"\n";
  _reply = ConvertReply::PbToReply(rsp.reply());
}
}

}