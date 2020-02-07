//
// Created by liuping on 2020/2/7.
//

#include <iostream>
#include <functional>

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
    std::cout << Redis().get("test_redis").value();
  }
};

AppBase<test_toml::Root> app;
int main() {
  app.Init("test.toml");

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