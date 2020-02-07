//
// Created by liuping on 2020/2/1.
//

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/app_base.h"
#include <iostream>
#include "app/proto/redis_cmd.pb.h"
#include "app/proto/cmd.pb.h"
#include "redissvr_toml.hpp"
#include "redis_client.h"


AppBase<redissvr_toml::Root> app;
Redis::RedisClient *redis_client;

struct TransRedisCmd : public RegisterSvrTrans<TransRedisCmd, proto::cmd::RedisSvrCmd::kREDIS_CMD_REQ, 10000> {
  TransRedisCmd() {}
  void Task(CoYield &co) override {
    MsgHead * msg = static_cast<MsgHead *>(co.GetMsg());
    proto::redis::RedisCmdReq req;
    proto::redis::RedisCmdRsp rsp;
    req.ParseFromArray(msg->Data(), msg->Size());
    try {
      Redis::RedisCmd cmd(*redis_client, co);
      std::vector<std::string> cmd_list;
      for (int i = 0; i < req.cmd_argv_size(); i++) {
        cmd_list.push_back(std::move(*req.mutable_cmd_argv(i)));
      }
      auto reply = cmd.Cmd(cmd_list);
      ReplyToPb(reply, *rsp.mutable_reply());
    } catch (const std::exception &e) {
      std::cout << "redis cmd error: " << e.what();
    }
    SendMsg(msg->msg_head_, rsp);
  }

  static void ReplyToPb(redisReply *reply, proto::redis::redisReply &pb_reply) {
    pb_reply.set_type(reply->type);
    pb_reply.set_integer(reply->integer);
    pb_reply.set_str(reply->str, reply->len);
    if (reply->type == REDIS_REPLY_ARRAY) {
      for (int i = 0; i < reply->elements; i++) {
        auto add_reply = pb_reply.add_element();
        ReplyToPb(reply->element[i], *add_reply);
      }
    }
  }
};

int main() {
  app.Init("redissvr.toml");
  redis_client = new Redis::RedisClient(app.EvBase());
  redis_client->Init(app.config.redis.ip, app.config.redis.port);
  app.Run();
}