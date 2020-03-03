//
// Created by liuping on 2020/1/2.
//

#include "mq_net.h"
#include "commlib/logging.h"
#include <amqpcpp/libevent.h>
#include <iostream>

class MyHandler : public AMQP::LibEventHandler {
  ClientBase *cbase_;
 public:
  MyHandler(event_base *ebase, ClientBase *cbase) : AMQP::LibEventHandler(ebase), cbase_(cbase) {}
  void onConnected(AMQP::TcpConnection *connection) final {
    INFO("amqp connected");
    cbase_->SetConnected();
  }
  void onClosed(AMQP::TcpConnection *connection) final {
    INFO("aqpb close");
    cbase_->SetDisConnected();
  }
};

void RabbitMQNet::SendMsg(proto::Msg::MsgHead &msg) {
  if (State() != ClientBase::kConnected) {
    WARNING("rabbitmq disconnected cmd:{},dst_bus_id: {}", msg.cmd(), msg.dst_bus_id());
    return;
  }
  if (!channel_send_->usable() || !recv_channel_succ_) {
    ERROR("msg send faild channel not usable msg: cmd:{},dst_bus_id: {}", msg.cmd(), msg.dst_bus_id());
    return;
  };
  if (msg.dst_bus_id().empty()) {
    ERROR("dst_bus_id empty:cmd {}", msg.cmd());
    return;
  }
  msg.set_src_bus_id(key_);
  channel_send_->publish(exchange_, msg.dst_bus_id(), msg.SerializeAsString());
}

RabbitMQNet::RabbitMQNet(event_base *evbase, std::string mq_addr, std::string exchange, std::string queue, std::string key) :
    ClientBase(),
    handler_(std::make_shared<MyHandler>(evbase, this)),
    exchange_(exchange),
    queue_(queue),
    key_(key),
    mq_addr_(mq_addr) {
  self_id_ = key;
}

void RabbitMQNet::Reconnect() {
  if (NotNeedReconnect()) {
    return;
  }
  SetConnecting();
  connection_ = std::make_shared<AMQP::TcpConnection>(handler_.get(), AMQP::Address(mq_addr_));
  channel_send_ = std::make_shared<AMQP::TcpChannel>(connection_.get());
  channel_consume_ = std::make_shared<AMQP::TcpChannel>(connection_.get());
  send_channel_succ_ = false;
  recv_channel_succ_ = false;

  channel_consume_->declareExchange(exchange_, AMQP::ExchangeType::direct).onSuccess([this](){
    INFO ("declare recv exchange success");
    this->recv_channel_succ_ = true;
  });

  // create a temporary queue
  channel_consume_->declareQueue(queue_, AMQP::exclusive).onSuccess([](const std::string &name,
                                                                                 uint32_t messagecount,
                                                                                 uint32_t consumercount) {
    // report the name of the temporary queue
    INFO("declared queue :{}", name);
  });
  channel_consume_->bindQueue(exchange_, queue_, key_);
  auto &consumeQ = channel_consume_->consume(queue_);
  consumeQ.onMessage([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
    if (this->recv_msg_handler_) {
      this->recv_msg_handler_(std::string(message.body(), message.bodySize()));
    }
  });
  consumeQ.onSuccess([this](const std::string &consumer){
    auto &send_channel = this->channel_send_->declareExchange(exchange_, AMQP::ExchangeType::direct);
    send_channel.onSuccess([this]() {
      INFO("declare send exchange success");
      this->send_channel_succ_ = true;
      if (this->send_channel_success_) {
        this->send_channel_success_();
      }
    });
  });
}

void RabbitMQNet::Heartbeat() {
  if (connection_ == nullptr) {
    return;
  }
  if(connection_->usable()) {
    connection_->heartbeat();
  }
}