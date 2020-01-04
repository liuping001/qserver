//
// Created by liuping on 2020/1/2.
//

#pragma once

#include <functional>
#include "net_handler.h"
#include "third/AMQP-CPP/include/amqpcpp.h"
#include "third/AMQP-CPP/include/amqpcpp/linux_tcp.h"

class RabbitMQNet : public NetHandler {
 public:
  void SendMsg(proto::Msg::MsgHead &msg) final;

  RabbitMQNet(AMQP::TcpConnection &connection, std::string exchange, std::string queue, std::string key);
 private:
  AMQP::TcpChannel channel_send_;
  AMQP::TcpChannel channel_consume_;
  std::string exchange_;
  std::string queue_;
  std::string key_;
};