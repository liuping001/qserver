//
// Created by liuping on 2020/1/2.
//

#include "mq_net.h"

#include <iostream>
void RabbitMQNet::SendMsg(proto::Msg::MsgHead &msg) {
  msg.set_src_bus_id(key_);
  std::cout <<"SendMsg:" << msg.ShortDebugString() <<"\n";
  channel_send_.publish(exchange_, msg.dst_bus_id(), msg.SerializeAsString());
}

RabbitMQNet::RabbitMQNet(AMQP::TcpConnection &connection, std::string exchange, std::string queue, std::string key) :
    channel_send_(&connection),
    channel_consume_(&connection),
    exchange_(exchange),
    queue_(queue),
    key_(key) {
  self_id_ = key;
  channel_consume_.declareExchange(exchange_, AMQP::ExchangeType::direct).onSuccess([](){
    std::cout << "declare exchange success" << std::endl;
  });

  // create a temporary queue
  channel_consume_.declareQueue(queue_, AMQP::exclusive).onSuccess([&connection](const std::string &name,
                                                                                   uint32_t messagecount,
                                                                                   uint32_t consumercount) {
    // report the name of the temporary queue
    std::cout << "declared queue " << name << std::endl;
  });
  channel_consume_.bindQueue(exchange_, queue_, key_);
  auto &consumeQ = channel_consume_.consume(queue_);
  consumeQ.onMessage([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
    if (this->recv_msg_handler_) {
      this->recv_msg_handler_(std::string(message.body(), message.bodySize()));
    }
  });

  channel_send_.declareExchange(exchange_, AMQP::ExchangeType::direct).onSuccess([](){
    std::cout << "declare exchange success" << std::endl;
  });
}