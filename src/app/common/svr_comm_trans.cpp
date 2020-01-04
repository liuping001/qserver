//
// Created by liuping on 2020/1/3.
//

#include "svr_comm_trans.h"

SvrCommTrans::SvrCommTrans(NetHandler *net)
  : net_(net) {

}

//void SvrCommTrans::DoTask(const CoYield &co) {
//
//}

void SvrCommTrans::SendMsg(const std::string & dst_svr_id, uint32_t cmd, const std::string & msg, uint32_t dst_co_id) {
  proto::Msg::MsgHead msg_head;
  msg_head.set_msg(msg);
  msg_head.set_cmd(cmd);
  msg_head.set_dst_co_id(dst_co_id);
  msg_head.set_dst_bus_id(dst_svr_id);
  net_->SendMsg(msg_head);
}

void SvrCommTrans::SendMsg(const proto::Msg::MsgHead &src_msg, const std::string &msg) {
  SendMsg(src_msg.src_bus_id(), src_msg.cmd(), msg, src_msg.src_co_id());
}

int SvrCommTrans::SendMsgThenYield(CoYield &co, const std::string & dst_svr_id, uint32_t cmd, const std::string & msg,
                           uint32_t dst_co_id) {
  proto::Msg::MsgHead msg_head;
  msg_head.set_msg(msg);
  msg_head.set_cmd(cmd);
  msg_head.set_src_co_id(co.co_id_);
  msg_head.set_dst_co_id(dst_co_id);
  msg_head.set_dst_bus_id(dst_svr_id);
  net_->SendMsg(msg_head);
  return co.Yield();
}