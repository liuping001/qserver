
#pragma once
#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>

#include "app/common/svr_comm_trans.h"
#include "app/common/mq_net.h"
#include "app/common/event_help.h"
#include "third/toml/cpptoml.h"

#include "commlib/logging.h"
#include "app/proto/common.pb.h"
#include "app/proto/cmd.pb.h"

#include "get_ip.h"
#include "svr_type.h"

template <class Config>
class AppBase {
 public:

  Config config;

  int Init(const std::string &svr_type, const std::string &config_path) {
    auto root = cpptoml::parse_file(config_path);
    config.FromToml(root);

    InitLog();

    auto self_id = svr_type + "-" + MakeSelfId();
    if (svr_type == kDiscoverySvr) {
      self_id = kDiscoverySvr;
    }

    evbase = event_base_new();
    mq_net = new RabbitMQNet(evbase, config.mq_addr, config.router, self_id, self_id);
    mq_net->SetRecvMsgHandler([](const std::string &msg) {
      MsgHead msg_head;
      msg_head.msg_head_.ParseFromString(msg);
      DEBUG("recv msg: cmd:{}, co_id:{}, src_bus_id: {}, src_co_id: {}",msg_head.Cmd(),msg_head.CoId(), msg_head.msg_head_.src_bus_id(), msg_head.msg_head_.src_co_id());
      TransMgr::get().OnMsg(msg_head);
    });

    if (svr_type != kDiscoverySvr) {
      mq_net->OnSendChannelSuccess([this]() {
        DEBUG("on send channel success");
        this->ReportSelf(true);
      });
      AddTimer(10 * 1000, [this]() { this->ReportSelf();}, true);
    }
    AddTimer(2 * 1000, std::bind(&RabbitMQNet::Reconnect, mq_net), true);
    AddTimer(30 * 1000, std::bind(&RabbitMQNet::Heartbeat, mq_net), true);

    SvrCommTrans::Init(mq_net);

    AddTimer(1, []() {
      TransMgr::get().TickTimeOutCo();
    }, true);

    return 0;
  }

  void AddTimer(uint32_t ms, std::function<void ()> cb, bool repeat = false) {
    EvAddTimer(evbase, ms, cb, repeat);
  }
  // run the loop
  void Run() {
    event_base_dispatch(evbase);

    event_base_free(evbase);
  }
  event_base &EvBase() {
    return *evbase;
  }
  void SendMsg(const std::string &svr_id, uint32_t cmd, const google::protobuf::Message &msg) {
    proto::Msg::MsgHead msg_head;
    msg_head.set_cmd(cmd);
    msg_head.set_dst_bus_id(svr_id);
    msg_head.set_msg(msg.SerializeAsString());
    DEBUG("{}:{}", msg.GetTypeName(), msg.ShortDebugString());
    mq_net->SendMsg(msg_head);
  }

 private:
  struct event_base *evbase;
  RabbitMQNet *mq_net;

 private:
  void InitLog() {
    logger_mgr::ConfigAsync config;
    config.log_path = "log";
    config.data_path = "log";
    config.level = spdlog::level::debug;
    logger_mgr::InitAsync(config);
    spdlog::flush_on(spdlog::level::debug);
  }

  std::string MakeSelfId() {
    auto ip_list = GetLocalIP();
    if (ip_list.empty()) {
      FATAL("IP LIST EMPTY");
    }
    return ip_list[0] + "-" + std::to_string(getpid());
  }
  void ReportSelf (bool init = false) {
    proto::common::SvrReportNotify notify;
    if (init) {
      notify.set_status(proto::common::SVR_JOIN);
    } else {
      notify.set_status(proto::common::SVR_ONLINE);
    }
    notify.set_svr_id(mq_net->self_id_);
    SendMsg(kDiscoverySvr, proto::cmd::CommonCmd::kSVR_REPORT_REQ, notify);
  }
};