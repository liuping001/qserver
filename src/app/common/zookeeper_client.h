// Copyright []
#pragma once
#include <cerrno>

#include <cstdio>
#include <string>
#include <vector>
#include <functional>

#include "zookeeper/include/zookeeper.h"
#include "zookeeper/include/zookeeper_log.h"
#include "commlib/co/co_task.h"
#include "commlib/client_base.h"

struct ZooKeeperClient : public ClientBase {
  using Watcher = std::function<void (int type, int state, const char *path)>;
  explicit ZooKeeperClient() {}
  ~ZooKeeperClient();
  bool Init(const std::string &host_info, uint32_t time_out);
  void Check();
  void Reconnect() final;
  void SetWatcherHandler(Watcher &&watcher) { watcher_ = std::move(watcher); }

  std::string host_info_;
  uint32_t time_out_ = 5000;
  zhandle_t *zkhandle_ = nullptr;
  Watcher watcher_;


  static void DefaultWatcher(zhandle_t *zkh, int type, int state, const char *path, void *context);
  static void DataCompletion(int rc, const char *value, int valueLen, const struct Stat *stat, const void *data);
  static void SVDataCompletion(int rc, const struct String_vector *strings, const void *data);
  static void SDataCompletion(int rc, const char *value, const void *data);
  static void StatCompletion(int rc, const struct Stat *stat, const void *data);
  static void EventLoopOnce(zhandle_t *zkh, uint32_t time_out_ms = 0);
};

struct ZooCmd {
  ZooCmd(ZooKeeperClient &client, CoYield &co) : client_(client), co_(co) {}
  ZooKeeperClient &client_;
  CoYield &co_;
  int Create(const std::string &path, const std::string &value, bool temp = false);
  bool Set(const std::string &path, const std::string &value);
  std::string Get(const std::string &path);
  std::string WGet(const std::string &path);
  std::vector<std::string> GetChildren(const std::string &path);
  std::vector<std::string> WGetChildRen(const std::string &path);
};
