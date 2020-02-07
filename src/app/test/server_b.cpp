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

struct SayHello : public RegisterSvrTrans<SayHello, 1001, 1000>  {
  SayHello() {}
  void Task(CoYield &co) override {
    proto::Test::SayHelloReq req;
    req.set_content("I am liuping");
    auto rsp = SendMsgRpc<proto::Test::SayHelloRsp>( "1.1.1.1", 1001, req);
  }
};

AppBase<server_b_toml::Root> app;
int main() {
  app.Init("server_b.toml");

  auto task = [&]() {
    app.AddTimer( 1, [](){
      MsgHead msg_head;
      msg_head.msg_head_.set_cmd(1001);
      TransMgr::get().OnMsg(msg_head);
    }, true);
  };
  app.AddTimer(2000, task);
  // run the loop
  app.Run();
}