// Copyright []

#include "test_base.hpp"
#include "app/common/zookeeper_client.h"
#include <unistd.h>
CoTask co_task;

TEST_F(connect) {
  const char *host_info = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
  zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);  // 设置log日志的输出级别
  ZooKeeperClient zk_client;
  auto b = zk_client.Init(host_info, 5000);
  if (b) {
    LOG_DEBUG(("init zk"));
  } else {
    LOG_DEBUG(("init failed zk"));
    exit(1);
  }
  co_task.DoTack([&zk_client](CoYield &co) {
    ZooCmd cmd(zk_client, co);
    cout << cmd.Get("/tasks/task-0000000000") << std::endl;
    auto root = cmd.GetChildren("/");
    for (const auto &item : root) {
      cout << " -> " << item << endl;
    }
    cmd.WGet("/tasks/task-0000000000");
    cout << "create /tt ret:" << cmd.Create("/tt", "");
    cout << "create /tt ret:" << cmd.Create("/tt/t1", "");
    cmd.WGet("/tt/t1");
    cmd.WGet("/tt");
  });

  for (int i = 0; i < 100; i++) {
    usleep(1000*1000);
    zk_client.Check();
  }
}

TEST_FINSH