//
// Created by liuping on 2020/1/5.
//

#include <iostream>
#include <functional>

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/app_base.h"
#include "app/proto/say_hello.pb.h"
#include "server_b_toml.hpp"
#include "commlib/time_mgr.h"

uint32_t seq = 0;
struct SayHello : public RegisterSvrTrans<SayHello, 1001, 10000>  {
  SayHello() {}
  void Task(CoYield &co) override {
    proto::Test::SayHelloReq req;
    req.set_content("I am liuping");
    req.set_seq(++seq);
    req.set_time_req(time_mgr::now_ms());
    try {
      auto rsp = SendMsgRpcByType<proto::Test::SayHelloRsp>("server_a", 1001, req);
      DEBUG("rsp: {}", rsp.ShortDebugString());
    } catch (const std::exception &e) {
      ERROR("exception:{} req:{}", e.what(), req.ShortDebugString());
    }
  }
};

AppBase<server_b_toml::Root> app;
int main() {
  app.Init("server_b", "server_b.toml");

  auto task = [&]() {
    app.AddTimer( 1, [](){
      for (int i = 0; i < 2; i++) {
        MsgHead msg_head;
        msg_head.msg_head_.set_cmd(1001);
        TransMgr::get().OnMsg(msg_head);
      }
    }, true);
  };
  app.AddTimer(2000, task);
  // run the loop
  app.Run();
}