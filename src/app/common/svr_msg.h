//
// Created by liuping on 2019/9/21.
//

#pragma once

#include "commlib/trans/trans_msg.h"

struct SvrMsgHead {
  uint32_t cmd;
  uint32_t co_id;
  uint32_t src_bus_id;
  uint32_t dst_bus_id;
  uint32_t data_size;
};

struct SvrMsg : public TransMsg {
  uint32_t Cmd() const { return MsgHead()->cmd; }
  uint32_t CoId() const { return MsgHead()->co_id; }
  const char *Data() const { return msg + sizeof(SvrMsgHead); }
  uint32_t Size() const { return MsgHead()->data_size; }
  SvrMsgHead *MsgHead() { return static_cast<SvrMsgHead *>(msg); }
 private:
  const char *msg;
};

