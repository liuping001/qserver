//
// Created by liuping on 2020/1/5.
//

#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/mq_net.h"

#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>
#include <event2/event_struct.h>
#include <iostream>

#include "say_hello.pb.h"

struct SayHello : public SvrCommTrans {
  SayHello(NetHandler *net) : SvrCommTrans(net) {}

  void DoTask(CoYield &co) override {
    proto::Test::SayHelloReq req;
    req.set_content("I am liuping");
    auto rsp = SendMsgRpc<proto::Test::SayHelloRsp>(co, "1.1.1.2", "1.1.1.1", 1001, req.SerializeAsString());
    std::cout << "SayHelloRsp:" << rsp.ShortDebugString() << "\n";
  }
};

struct event ev;
struct timeval tv;

uint32_t count = 0;
void time_cb(int fd, short event, void *argc) {
  TransMgr::get().TickTimeOutCo();
  event_add(&ev, &tv); // reschedule timer
  count++;

  if (count == 2000) {
    MsgHead msg_head;
    msg_head.msg_head_.set_cmd(1001);
    TransMgr::get().OnMsg(msg_head);
  }
}

int main() {
  // access to the event loop
  auto evbase = event_base_new();
  // handler for libevent
  AMQP::LibEventHandler handler(evbase);
  // make a connection
  AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://liuping:liuping@192.168.1.201"));

  RabbitMQNet mq_net(connection, "router1", "Q:1.1.1.2", "1.1.1.2");
  mq_net.SetRecvMsgHandler([](const std::string &msg) {
    MsgHead msg_head;
    msg_head.msg_head_.ParseFromString(msg);
    std::cout << "MsgHead:" << msg_head.msg_head_.ShortDebugString() <<"\n";
    TransMgr::get().OnMsg(msg_head);
  });

  TransMgr::get().RegisterCmd<SayHello>(1001, 1, &mq_net);

  tv.tv_sec = 0;
  tv.tv_usec = 1000;

  event_assign(&ev, evbase, -1, 0, time_cb, NULL);
  event_add(&ev, &tv);

  // run the loop
  event_base_dispatch(evbase);

  event_base_free(evbase);

}