//
// Created by liuping on 2020/1/3.
//

#include "svr_comm_trans.h"
#include "redis_connect.h"

void SvrCommTrans::Init(NetHandler *net) {
  net_ = net;
}

void SvrCommTrans::DoTask(CoYield &co) {
  sw::redis::RedisConnect connect(*this);
  sw::redis::Redis redis(&connect);
  _co = &co;
  _redis_handler = &redis;
  Task(co);
  _redis_handler = nullptr;
  _co = nullptr;
}

void SvrCommTrans::SendMsg(const std::string & dst_svr_id, uint32_t cmd, const google::protobuf::Message &msg, uint32_t dst_co_id) {
  proto::Msg::MsgHead msg_head;
  msg_head.set_msg(msg.SerializeAsString());
  msg_head.set_cmd(cmd);
  msg_head.set_dst_co_id(dst_co_id);
  msg_head.set_dst_bus_id(dst_svr_id);
  net_->SendMsg(msg_head);
}

void SvrCommTrans::SendMsg(const proto::Msg::MsgHead &src_msg, const google::protobuf::Message &msg) {
  SendMsg(src_msg.src_bus_id(), src_msg.cmd() + 1, msg, src_msg.src_co_id());
}

void SvrCommTrans::SendMsgBySvrTye(uint32_t type, uint32_t cmd, const google::protobuf::Message &msg, uint32_t dst_co_id) {
  SendMsg(std::to_string(type) + ".1", cmd, msg, dst_co_id);
}

int SvrCommTrans::SendMsgThenYield(const std::string & src_svr_id, const std::string & dst_svr_id, uint32_t cmd, const google::protobuf::Message &msg,
                           uint32_t dst_co_id) {
  proto::Msg::MsgHead msg_head;
  msg_head.set_msg(msg.SerializeAsString());
  msg_head.set_cmd(cmd);
  msg_head.set_src_co_id((*_co).co_id_);
  msg_head.set_dst_co_id(dst_co_id);
  msg_head.set_src_bus_id(src_svr_id);
  msg_head.set_dst_bus_id(dst_svr_id);
  net_->SendMsg(msg_head);
  return Yield((*_co));
}

NetHandler * SvrCommTrans::net_ = nullptr;

sw::redis::Redis& SvrCommTrans::Redis() {
  return *_redis_handler;
}