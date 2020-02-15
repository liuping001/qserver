//
// Created by liuping on 2020/2/15.
//

#include "svr_list.h"

#include "app/common/svr_comm_trans.h"
#include "app/proto/cmd.pb.h"
#include "app/proto/common.pb.h"
#include "commlib/logging.h"

// 处理全量
struct TransSvrReport : public RegisterSvrTrans<TransSvrReport, proto::cmd::CommonCmd::kSVR_REPORT_RSP> {
  TransSvrReport() {}

  void Task(CoYield &co) override {
    auto msg_head = GetMsg();
    proto::common::SvrReportNotifyRsp rsp;
    rsp.ParseFromString(msg_head.msg());
    DEBUG("{} : {}",rsp.GetTypeName(), rsp.ShortDebugString());
    svr_list::get().Clear();
    for (auto &item : rsp.svr_list()) {
     svr_list::get().AddSvr(item);
    }
  }
};

// 处理增量
struct TransSvrReportReq: public RegisterSvrTrans<TransSvrReportReq, proto::cmd::CommonCmd::kSVR_REPORT_REQ> {
  TransSvrReportReq() {}
  void Task(CoYield &co) override {
    auto msg_head = GetMsg();
    proto::common::SvrReportNotify notify;
    notify.ParseFromString(msg_head.msg());
    DEBUG("{} : {}",notify.GetTypeName(), notify.ShortDebugString());
    if (notify.status() > 0) {
      svr_list::get().AddSvr(notify.svr_id());
    } else {
      svr_list::get().RemoveSvr(notify.svr_id());
    }
  }

};

void SvrList::AddSvr(const std::string &svr_id) {
  if (svr_set.find(svr_id) != svr_set.end()) {
    return;
  }

  auto type = GetType(svr_id);
  if (type.empty()) {
    return;
  }

  svr_by_type[type].push_back(svr_id);
  svr_set.insert(svr_id);
}

void SvrList::RemoveSvr(const std::string &svr_id) {
  if (svr_set.find(svr_id) == svr_set.end()) {
    return;
  }
  auto list = GetSvrListByType(GetType(svr_id));
  if (list == nullptr) {
    return;
  }
  for (auto iter = list->begin(); iter != list->end(); iter++) {
    if (*iter != svr_id) {
      continue;
    }
    list->erase(iter);
    break;
  }
  svr_set.erase(svr_id);
}

std::string SvrList::GetType (const std::string &svr_id) {
  auto pos = svr_id.find_first_of('-');
  if (pos == svr_id.npos) {
    return {};
  }
  return svr_id.substr(0, pos);
}

std::vector<std::string> *SvrList::GetSvrListByType(const std::string &type) {
  auto iter = svr_by_type.find(type);
  if (iter == svr_by_type.end()) {
    return nullptr;
  }
  return &iter->second;
}