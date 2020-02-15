//
// Created by liuping on 2020/2/1.
//

#include <map>
#include <set>
#include "app/common/svr_comm_trans.h"
#include "commlib/trans/trans_mgr.h"
#include "app/common/app_base.h"
#include "app/proto/cmd.pb.h"
#include "discovery_toml.hpp"

#include "commlib/logging.h"
#include "commlib/time_mgr.h"
#include "app/proto/common.pb.h"
AppBase<discovery_toml::Root> app;
std::map<std::string, uint64_t> heartbeat_svrs;

void CheckHeartBeat() {
  auto now = time_mgr::now();
  for (auto iter = heartbeat_svrs.begin(); iter != heartbeat_svrs.end();) {
    if (iter->second + 13 < now) {
      proto::common::SvrReportNotify notify;
      notify.set_status(proto::common::SVR_OFFLINE);
      notify.set_svr_id(iter->first);
      for (auto &item : heartbeat_svrs) {
        if (item.first == iter->first) {
          continue;
        }
        app.SendMsg(item.first,proto::cmd::CommonCmd::kSVR_REPORT_REQ, notify);
      }
      iter = heartbeat_svrs.erase(iter);
    } else {
      iter ++;
    }
  }
}

struct TransSvrReport : public RegisterSvrTrans<TransSvrReport, proto::cmd::CommonCmd::kSVR_REPORT_REQ> {
   TransSvrReport() {}
  void Task(CoYield &co) override {
    auto msg_head = GetMsg();
    proto::common::SvrReportNotify notify;
    notify.ParseFromString(msg_head.msg());
    DEBUG("{} : {}",notify.GetTypeName(), notify.ShortDebugString());

    if (notify.status() > 0) {
      auto now = time_mgr::now();
      heartbeat_svrs[notify.svr_id()] = now;
    }

    if (notify.status() == proto::common::SVR_JOIN) {
      proto::common::SvrReportNotifyRsp rsp;
      for (auto &item : heartbeat_svrs) {
        if (item.first == notify.svr_id()) {
          continue;
        }
        rsp.add_svr_list(item.first);
      }
      SendMsg(msg_head.src_bus_id(), proto::cmd::CommonCmd::kSVR_REPORT_RSP, rsp);
    }

    if (notify.status() != proto::common::SVR_ONLINE) {
      for (auto &item : heartbeat_svrs) {
        if (item.first == notify.svr_id()) {
          continue;
        }
        SendMsg(item.first, proto::cmd::CommonCmd::kSVR_REPORT_REQ, notify);
      }
    }
  }
};

int main() {
  app.Init(kDiscoverySvr, "discovery.toml");
  app.AddTimer(2 * 1000, CheckHeartBeat, true);
  app.Run();
}