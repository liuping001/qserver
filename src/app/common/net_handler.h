//
// Created by liuping on 2020/1/2.
//

#pragma once

#include "commlib/trans/trans_msg.h"
#include "msg_head.pb.h"

struct NetHandler {

  virtual void SendMsg(proto::Msg::MsgHead &msg) = 0;

  void SetRecvMsgHandler(std::function<void (const std::string &)> recv_msg_handler) {
    recv_msg_handler_ = std::move(recv_msg_handler);
  }
  virtual ~NetHandler() {}

  std::function<void (const std::string &)> recv_msg_handler_;
};

