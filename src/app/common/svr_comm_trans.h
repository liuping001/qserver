//
// Created by liuping on 2020/1/3.
//

#pragma once

#include "commlib/trans/trans_mgr.h"
#include "net_handler.h"
#include "app/proto/msg_head.pb.h"
#include "commlib/redis/redis.h"

struct MsgHead : public TransMsg {
  uint32_t Cmd() const final { return msg_head_.cmd(); }
  uint32_t CoId() const final { return msg_head_.dst_co_id(); }
  const char *Data() const { return msg_head_.msg().c_str(); }
  uint32_t Size() const { return msg_head_.msg().size(); }
  proto::Msg::MsgHead msg_head_;
};

class SvrCommTrans : public Trans {
 public:
  // SendMsg 不需要回复
  void SendMsg(const std::string & dst_svr_id, uint32_t cmd, const google::protobuf::Message & msg, uint32_t dst_co_id = 0);
  void SendMsg(const proto::Msg::MsgHead &src_msg, const google::protobuf::Message & msg);
  void SendMsgBySvrType(const std::string &type, uint32_t cmd, const google::protobuf::Message & msg, uint32_t dst_co_id = 0);

  // Rpc
  template <class Rsp>
  Rsp SendMsgRpc(const std::string & dst_svr_id, uint32_t cmd, const google::protobuf::Message &msg, uint32_t dst_co_id = 0);
  template <class Rsp>
  Rsp SendMsgRpcByType(const std::string &type, uint32_t cmd, const google::protobuf::Message & msg, uint32_t dst_co_id = 0);
  template <class Rsp>
  Rsp SendMsgRpc(const proto::Msg::MsgHead &src_msg, const google::protobuf::Message & msg);

  static void Init(NetHandler *net);
  static NetHandler *net_;

 protected:
  void DoTask(CoYield &co) final;
  virtual void Task(CoYield &co) = 0;
  sw::redis::Redis& Redis();

  proto::Msg::MsgHead GetMsg() {
    return std::move(static_cast<MsgHead *>(const_cast<void *>(_co->GetMsg()))->msg_head_); // 使用右值移动栈上面的变量
  }
 private:
  sw::redis::Redis *_redis_handler;
  CoYield *_co;

 private:
  int SendMsgThenYield(const std::string & src_svr_id, const std::string & dst_svr_id, uint32_t cmd,
                       const google::protobuf::Message &msg, uint32_t dst_co_id);
  std::string GetOneSvrByType(const std::string &type);
};

template <class Rsp>
Rsp SvrCommTrans::SendMsgRpc(const std::string &dst_svr_id,
    uint32_t cmd,
    const google::protobuf::Message &msg,
    uint32_t dst_co_id) {
  auto ret = SendMsgThenYield(net_->self_id_, dst_svr_id, cmd, msg, dst_co_id);
  if (ret != 0) {
    throw std::runtime_error("time out");
  }
  const MsgHead *ret_msg = static_cast<const MsgHead*>((*_co).GetMsg());
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
Rsp SvrCommTrans::SendMsgRpc(const proto::Msg::MsgHead &src_msg, const google::protobuf::Message &msg) {
  return SendMsgRpc<Rsp>(src_msg.dst_bus_id(), src_msg.src_bus_id(), src_msg.cmd() + 1, msg, src_msg.dst_co_id());
}

template <class Rsp>
Rsp SvrCommTrans::SendMsgRpcByType(const std::string &type, uint32_t cmd, const google::protobuf::Message &msg, uint32_t dst_co_id) {
  return SendMsgRpc<Rsp>(GetOneSvrByType(type), cmd, msg, dst_co_id);
}


template<class T, uint32_t cmd, uint16_t count = 1>
class RegisterSvrTrans : public SvrCommTrans {
 public:
  RegisterSvrTrans() : SvrCommTrans() {
    reg;
  }
 private:
  static bool reg;
};


template<class T, uint32_t cmd, uint16_t count>
bool RegisterSvrTrans<T, cmd, count>::reg = TransMgr::get().RegisterCmd<T>(cmd, count);