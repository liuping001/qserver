
#pragma once
#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>

#include "app/common/svr_comm_trans.h"
#include "app/common/mq_net.h"
#include "app/common/event_help.h"
#include "third/toml/cpptoml.h"
template <class Config>
class AppBase {
 public:

  Config config;

  int Init(const std::string &config_path) {
    auto root = cpptoml::parse_file(config_path);
    config.FromToml(root);
    evbase = event_base_new();
    handler = new AMQP::LibEventHandler(evbase);
    connection = new AMQP::TcpConnection(handler,AMQP::Address(config.mq_addr));
    mq_net = new RabbitMQNet(*connection, config.router, config.self_id, config.self_id);
    mq_net->SetRecvMsgHandler([](const std::string &msg) {
      MsgHead msg_head;
      msg_head.msg_head_.ParseFromString(msg);
      std::cout << "MsgHead:" << msg_head.msg_head_.ShortDebugString() << "\n";
      TransMgr::get().OnMsg(msg_head);
    });
    SvrCommTrans::Init(mq_net);

    AddTimer(1, []() {
      TransMgr::get().TickTimeOutCo();
    }, true);

    AddTimer(30 * 1000, [this](){
      if(this->connection->usable()) {
        this->connection->heartbeat();
      }
    },true);
  }

  void AddTimer(uint32_t ms, std::function<void ()> cb, bool repeat = false) {
    EvAddTimer(evbase, ms, cb, repeat);
  }
  // run the loop
  void Run() {
    event_base_dispatch(evbase);

    event_base_free(evbase);
  }
  event_base &EvBase() {
    return *evbase;
  }
 private:
  struct event_base *evbase;
  RabbitMQNet *mq_net;
  AMQP::LibEventHandler *handler;
  AMQP::TcpConnection *connection;
};