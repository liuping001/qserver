//
// Created by liuping on 2020/2/7.
//

#include <iostream>
#include <functional>
#include <unordered_map>

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/app_base.h"
#include "app/proto/cmd.pb.h"

#include "test_toml.hpp"
const int kTestRedisCmd = 10000;

struct TestRedis : public RegisterSvrTrans<TestRedis, kTestRedisCmd, 1>  {
  TestRedis() {}
  void Task(CoYield &co) override {
    Redis().set("test_redis", "It's ok");
    std::cout << "Redis().get:"<<Redis().get("test_redis").value() <<"\n";
    std::cout<< "Redis().incr:" << Redis().incr("incr") <<"\n";
    std::cout<< "Redis().incr:" << Redis().incr("incr") <<"\n";
    std::cout<< "Redis().zadd:" << Redis().zadd("rank", "liuping", 199) <<"\n";
    std::cout<< "Redis().zadd:" << Redis().zadd("rank", "lisi", 100) <<"\n";
    std::unordered_map<std::string, double> with_score;
    Redis().zrange("rank",0,-1, std::inserter(with_score, with_score.end()));
    for (auto &item : with_score) {
      std::cout << item.first << " " << item.second <<"\n";
    }
  }
};

AppBase<test_toml::Root> app;
int main() {
  app.Init("test", "test.toml");

  auto task = [&]() {
    app.AddTimer( 1, [](){
      MsgHead msg_head;
      msg_head.msg_head_.set_cmd(kTestRedisCmd);
      TransMgr::get().OnMsg(msg_head);
    });
  };
  app.AddTimer(2000, task);
  // run the loop
  app.Run();
}