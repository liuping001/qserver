
#pragma once
#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>
#include <functional>

#include "app/common/svr_comm_trans.h"
#include "app/common/mq_net.h"
#include "app/common/event_help.h"
#include "third/toml/cpptoml.h"

#include "commlib/logging.h"
#include "app/proto/common.pb.h"
#include "app/proto/cmd.pb.h"

#include "get_ip.h"
#include "svr_type.h"
#include "zookeeper_client.h"
#include "commlib/co/co_task.h"
#include "svr_list.h"
#include "commlib/time_mgr.h"

void GetServers(CoYield &co, ZooKeeperClient *zk_client, const char *path) {
  ZooCmd cmd(*zk_client, co);
  auto root = cmd.GetChildren(path);

  svr_list::get().Clear();
  for (const auto &item : root) {
    svr_list::get().AddSvr(item);
    INFO("server name :{}", item);
  }
  cmd.WGetChildRen("/online_servers");
}
template <class Config>
class AppBase {
 public:
  ~AppBase() {
    if (evbase) { event_base_free(evbase); }
  }
  Config config_;

  int Init(const std::string &svr_type, const std::string &config_path) {
    auto root = cpptoml::parse_file(config_path);
    config_.FromToml(root);

    InitLog();

    auto self_id = svr_type + "-" + MakeSelfId();
    if (svr_type == kDiscoverySvr) {
      self_id = kDiscoverySvr;
    }

    // init MQ
    evbase = event_base_new();
    mq_net = new RabbitMQNet(evbase, config_.mq_addr, config_.router, self_id, self_id);
    mq_net->SetRecvMsgHandler([](const std::string &msg) {
      MsgHead msg_head;
      msg_head.msg_head_.ParseFromString(msg);
      DEBUG("recv msg: cmd:{}, co_id:{}, src_bus_id: {}, src_co_id: {}", msg_head.Cmd(), msg_head.CoId(),
      msg_head.msg_head_.src_bus_id(), msg_head.msg_head_.src_co_id());
      TransMgr::get().OnMsg(msg_head);
    });
    mq_net->Reconnect();
    AddTimer(2 * 1000, std::bind(&RabbitMQNet::Reconnect, mq_net), true);
    AddTimer(30 * 1000, std::bind(&RabbitMQNet::Heartbeat, mq_net), true);
    SvrCommTrans::Init(mq_net);

    AddTimer(1, []() {
      TransMgr::get().TickTimeOutCo();
    }, true);

    // init zk
    zk_client_ = std::make_shared<ZooKeeperClient>();
    auto zk_init = zk_client_->Init("127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183", 5000);
    if (!zk_init) {
      return -1;
    }
    auto &cotask = co_task;
    zk_client_->SetWatcherHandler(std::bind(&AppBase<Config>::Watcher, this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
    AddTimer(100, [this, &cotask]() {
      auto now = time_mgr::now_ms();
      cotask.DoTimeOutTask(now);
    }, true);
    AddTimer(100, std::bind(&ZooKeeperClient::Check, zk_client_.get()), true);

    // report self
    if (svr_type != kDiscoverySvr) {
      auto zk_client = zk_client_;
      co_task.DoTack([zk_client, self_id](CoYield &co){
        ZooCmd cmd(*zk_client, co);
        cmd.WGetChildRen("/online_servers");
        auto ret = cmd.Create("/online_servers/" + self_id, "", true);
        INFO("make self id ret:{}, self_id:{}", ret, self_id);
      });
    }

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
  std::shared_ptr<ZooKeeperClient> zk_client_;
  CoTask co_task;

 private:
  void InitLog() {
    logger_mgr::ConfigAsync config;
    config.log_path = "log";
    config.data_path = "log";
    config.level = spdlog::level::level_enum(config_.log.log_level);
    config.max_files = config_.log.log_remain_size;
    logger_mgr::InitAsync(config);
    spdlog::flush_on(spdlog::level::debug);
    INFO("init log finish. level:{}, log_path:{}", config.level, config.log_path);
  }

  std::string MakeSelfId() {
    auto ip_list = GetLocalIP();
    if (ip_list.empty()) {
      FATAL("IP LIST EMPTY");
    }
    return ip_list[0] + "-" + std::to_string(getpid());
  }
  void Watcher(int type, int state, const char *path) {
      if (type == ZOO_CHILD_EVENT) {
        co_task.DoTack(std::bind(GetServers, std::placeholders::_1, zk_client_.get(), path));
    }
  }
};