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

#include "commlib/logging.h"

AppBase<redissvr_toml::Root> app;
Redis::RedisClient *redis_client;

struct TransRedisCmd : public RegisterSvrTrans<TransRedisCmd, proto::cmd::RedisSvrCmd::kREDIS_CMD_REQ, 10000> {
  TransRedisCmd() {}
  void Task(CoYield &co) override {
    auto msg_head = GetMsg();
    proto::redis::RedisCmdReq req;
    proto::redis::RedisCmdRsp rsp;
    req.ParseFromString(msg_head.msg());
    try {
      redisReply *reply = nullptr;
      Redis::RedisCmd cmd(*redis_client, co);
      if (!req.formatted_cmd().empty()) {
        reply = cmd.FormattedCmd(req.formatted_cmd());
      } else {
        std::vector<std::string> cmd_list;
        for (int i = 0; i < req.cmd_argv_size(); i++) {
          cmd_list.push_back(std::move(*req.mutable_cmd_argv(i)));
        }
        reply = cmd.Cmd(cmd_list);
      }
      if (reply) {
        ReplyToPb(reply, *rsp.mutable_reply());
      } else {
        // todo
      }
    } catch (const std::exception &e) {
      ERROR("redis cmd error:{}", e.what());
    }
    SendMsg(msg_head, rsp);
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
  app.Init(kRedissvr, "redissvr.toml");
  redis_client = new Redis::RedisClient(app.EvBase());
  redis_client->Init(app.config.redis.ip, app.config.redis.port);
  app.AddTimer(1000, std::bind(&Redis::RedisClient::Reconnect, redis_client), true);
  app.Run();
}