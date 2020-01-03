//
// Created by liuping on 2020/1/2.
//

#include "mq_net.h"


void RabbitMQNet::SendMsg(proto::Msg::MsgHead &msg) {
  msg.set_src_bus_id(key_);
  channel_.publish(exchange_, msg.dst_bus_id(), msg.SerializeAsString());
}

RabbitMQNet::RabbitMQNet(AMQP::TcpConnection &connection, std::string exchange, std::string queue, std::string key) :
    channel_send_(&connection),
    channel_consume_(&connection),
    exchange_(exchange),
    queue_(queue),
    key_(key) {
  channel_consume_.bindQueue(exchange, queue, key);
  auto &consumeQ = channel_consume_.consume(queue);
  consumeQ.onMessage([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
    if (this->recv_msg_handler_) {
      this->recv_msg_handler_(std::string(message.body(), message.bodySize()));
    }
  });
}