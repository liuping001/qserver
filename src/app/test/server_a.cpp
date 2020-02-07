//
// Created by liuping on 2020/1/5.
//

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/mq_net.h"
#include "app/common/app_base.h"
#include <iostream>
#include "app/proto/say_hello.pb.h"
#include "server_a_toml.hpp"

struct SayHello : public RegisterSvrTrans<SayHello, 1001, 1> {
  SayHello() {}
  void Task(CoYield &co) override {
    MsgHead * msg = static_cast<MsgHead *>(co.GetMsg());
    proto::Test::SayHelloReq req;
    proto::Test::SayHelloRsp rsp;
    req.ParseFromArray(msg->Data(), msg->Size());
    rsp.set_content("hello : " + req.content());
    std::cout << "SayHelloReq: " << req.ShortDebugString() << "\n";
    std::cout << "SayHelloRsp: " << rsp.ShortDebugString() << "\n";
    SendMsg(msg->msg_head_, rsp);
  }
};

AppBase<server_a_toml::Root> app;
int main() {
  app.Init("server_b.toml");
  app.Run();
}