//
// Created by liuping on 2020/1/3.
//

#pragma once

#include "commlib/trans/trans.h"
#include "net_handler.h"
#include "msg_head.pb.h"

struct MsgHead : public TransMsg {
  uint32_t Cmd() const final { return msg_head_.cmd(); }
  uint32_t CoId() const final { return msg_head_.src_co_id(); }
  const char *Data() const { return msg_head_.msg().c_str(); }
  uint32_t Size() const { return msg_head_.msg().size(); }
  proto::Msg::MsgHead msg_head_;
};

class SvrCommTrans : public Trans {
 public:
  SvrCommTrans(NetHandler *net);

  void SendMsg(const std::string & dst_svr_id, uint32_t cmd, const std::string & msg, uint32_t dst_co_id = 0);
  void SendMsg(const proto::Msg::MsgHead &src_msg, const std::string & msg);

  template <class Rsp>
  Rsp SendMsgRpc(const CoYield &co, const std::string & dst_svr_id, uint32_t cmd, const std::string & msg, uint32_t dst_co_id = 0);
  template <class Rsp>
  Rsp SendMsgRpc(const CoYield &co, const proto::Msg::MsgHead &src_msg, const std::string & msg);

 protected:
//  void DoTask(const CoYield &co) override;

 private:
  int SendMsgThenYield(const CoYield &co, const std::string & dst_svr_id, uint32_t cmd, const std::string & msg,
                  uint32_t dst_co_id);
  NetHandler *net_;
};

template <class Rsp>
Rsp SvrCommTrans::SendMsgRpc(const CoYield &co,
                           const std::string &dst_svr_id,
                           uint32_t cmd,
                           const std::string &msg,
                           uint32_t dst_co_id) {
  auto ret = SendMsgThenYield(co, dst_svr_id, cmd, msg, dst_co_id);
  if (ret != 0) {
    throw std::runtime_error("time out");
  }
  MsgHead *ret_msg = static_cast<MsgHead*>(co.GetMsg());
  if (ret_msg == nullptr) {
    throw std::runtime_error("nullptr msg");
  }
  Rsp rsp;
  if(!rsp.ParseFromArray(ret_msg->Data(), ret_msg->Size())) {
    throw std::runtime_error("parse pb failed");
  }
  return rsp;
}

template <class Rsp>
Rsp SvrCommTrans::SendMsgRpc(const CoYield &co, const proto::Msg::MsgHead &src_msg, const std::string &msg) {
  return SendMsgRpc<Rsp>(co, src_msg.src_bus_id(), src_msg.cmd() + 1, msg, src_msg.dst_co_id());
}