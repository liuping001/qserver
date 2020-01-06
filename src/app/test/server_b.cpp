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
#include <functional>
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

struct EventInfo {
  struct event ev;
  struct timeval tv;
  std::function<void ()> cb;
  bool repeat = false;

  static void CB(int fd, short event, void *argc) {
    EventInfo *info = static_cast<EventInfo*>(argc);
    info->cb();
    if (info->repeat) {
      event_add(&info->ev, &info->tv);
    }
  }
};

void AddTimer(struct event_base * evbase, uint32_t ms, std::function<void ()> cb, bool repeat = false) {
  auto info = new EventInfo();
  auto sec = ms / 1000;
  auto usec = ms % 1000;
  info->tv.tv_sec = sec;
  info->tv.tv_usec = usec * 1000;
  info->cb = std::move(cb);
  info->repeat = repeat;

  event_assign(&info->ev, evbase, -1, 0, EventInfo::CB, info);
  event_add(&info->ev, &info->tv);
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

  TransMgr::get().RegisterCmd<SayHello>(1001, 1000, &mq_net);

  AddTimer(evbase, 1, []() {
    TransMgr::get().TickTimeOutCo();
  }, true);

  auto task = [=]() {
    AddTimer(evbase, 1, [](){
      MsgHead msg_head;
      msg_head.msg_head_.set_cmd(1001);
      TransMgr::get().OnMsg(msg_head);
    }, true);
  };
  AddTimer(evbase, 2000, task);
  // run the loop
  event_base_dispatch(evbase);

  event_base_free(evbase);

}