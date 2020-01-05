//
// Created by liuping on 2020/1/5.
//

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/mq_net.h"

#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>

#include <iostream>
#include "say_hello.pb.h"
struct SayHello : public SvrCommTrans {
  SayHello(NetHandler *net) : SvrCommTrans(net) {}
  void DoTask(CoYield &co) override {
    MsgHead * msg = static_cast<MsgHead *>(co.GetMsg());
    proto::Test::SayHelloReq req;
    proto::Test::SayHelloRsp rsp;
    req.ParseFromArray(msg->Data(), msg->Size());
    rsp.set_content("hello : " + req.content());
    std::cout << "SayHelloReq: " << req.ShortDebugString() << "\n";
    std::cout << "SayHelloRsp: " << rsp.ShortDebugString() << "\n";
    SendMsg(msg->msg_head_, rsp.SerializeAsString());
  }
};

int main() {
  // access to the event loop
  auto evbase = event_base_new();
  // handler for libevent
  AMQP::LibEventHandler handler(evbase);
  // make a connection
  AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://liuping:liuping@192.168.1.201"));

  RabbitMQNet mq_net(connection, "router1", "Q:1.1.1.1", "1.1.1.1");
  mq_net.SetRecvMsgHandler([](const std::string &msg) {
    MsgHead msg_head;
    msg_head.msg_head_.ParseFromString(msg);
    std::cout << "MsgHead:" << msg_head.msg_head_.ShortDebugString() <<"\n";
    TransMgr::get().OnMsg(msg_head);
  });

  TransMgr::get().RegisterCmd<SayHello>(1001, 1, &mq_net);

  // run the loop
  event_base_dispatch(evbase);

  event_base_free(evbase);

}