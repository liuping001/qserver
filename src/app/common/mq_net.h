//
// Created by liuping on 2020/1/2.
//

#pragma once

#include <functional>
#include <memory>
#include "net_handler.h"
#include "third/AMQP-CPP/include/amqpcpp.h"
#include "third/AMQP-CPP/include/amqpcpp/linux_tcp.h"
#include "commlib/client_base.h"

struct event_base;
namespace AMQP {
class LibEventHandler;
}
class RabbitMQNet : public NetHandler, public ClientBase {
 public:
  void SendMsg(proto::Msg::MsgHead &msg) final;
  RabbitMQNet(event_base *evbase, std::string mq_addr, std::string exchange, std::string queue, std::string key);
  void Reconnect() final;
  bool IsUsable() { return send_channel_succ_ && recv_channel_succ_; }
  void Heartbeat();
 private:
  std::shared_ptr<AMQP::LibEventHandler> handler_;
  std::shared_ptr<AMQP::TcpChannel> channel_send_;
  std::shared_ptr<AMQP::TcpChannel> channel_consume_;
  std::shared_ptr<AMQP::TcpConnection> connection_;
  std::string exchange_;
  std::string queue_;
  std::string key_;
  std::string mq_addr_;
  bool send_channel_succ_ = false;
  bool recv_channel_succ_ = false;
};