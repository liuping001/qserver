//
// Created by liuping on 2020/1/2.
//

#pragma once

#include "commlib/trans/trans_msg.h"
#include "app/proto/msg_head.pb.h"

struct NetHandler {

  virtual void SendMsg(proto::Msg::MsgHead &msg) = 0;

  void SetRecvMsgHandler(std::function<void (const std::string &)> recv_msg_handler) {
    recv_msg_handler_ = std::move(recv_msg_handler);
  }

  void OnSendChannelSuccess (std::function<void ()> send_channel_success) {
    send_channel_success_ = std::move(send_channel_success);
  }

  virtual ~NetHandler() {}

  std::function<void (const std::string &)> recv_msg_handler_;
  std::function<void ()> send_channel_success_;
  std::string self_id_;
};

