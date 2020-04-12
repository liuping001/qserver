#include <cstdio>
#include <errno.h>
#include "test_base.hpp"

#include "zookeeper/include/zookeeper.h"

static int connected = 0;
static int expired = 0;

struct MainWatch {
static void Watcher(zhandle_t* zkh,
                  int type,
                  int state,
                  const char* path,
                  void* context) {
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      connected = 1;//在接收到ZOO_CONNECTED_STATE事件后设置状态为已连接状态
      cout << "ZOO_CONNECTED_STATE\n";
    } else if (state == ZOO_CONNECTING_STATE) {
      connected = 0;
      cout << "ZOO_CONNECTING_STATE\n";
    } else if (state == ZOO_EXPIRED_SESSION_STATE) {
      expired = 1;//在接收到ZOO_EXPIRED_SESSION_STATE事件后设置为过期状态（并关闭会话句柄）
      connected = 0;
      cout << "ZOO_EXPIRED_SESSION_STATE\n";  
      zookeeper_close(zkh);
    }
}
}
static int EventLoopOnce(zhandle_t* zkh) {
    int fd, interest, events;
    fd_set rfds, wfds, efds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    struct timeval tv ={};
    tv.tv_sec = 5;

    // 注册zookeeper中的事件
    zookeeper_interest(zkh, &fd, &interest, &tv);
    if (fd != -1) {
        if (interest & ZOOKEEPER_READ) {
            FD_SET(fd, &rfds);
        } else {
            FD_CLR(fd, &rfds);
        }

        if (interest & ZOOKEEPER_WRITE) {
            FD_SET(fd, &wfds);
        } else {
            FD_CLR(fd, &wfds);
        }
    } else {
        fd = 0;
    }

    if (select(fd + 1, &rfds, &wfds, &efds, &tv) < 0) {
        // printf("zk loop select failed, err=%d, msg=%s", errno, strerror(errno));
    }
    events = 0;
    if (FD_ISSET(fd, &rfds)) {
        events |= ZOOKEEPER_READ;
    }
    if (FD_ISSET(fd, &wfds)) {
        events |= ZOOKEEPER_WRITE;
    }

    // 通知zookeeper客户端事件已经发生
    zookeeper_process(zkh, events);
    return 0;
}
};

TEST_F(connect) {
    const char * host_info = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);//设置log日志的输出级别
    auto zk = zookeeper_init(host_info, MainWatch::Watcher, 5000, nullptr, nullptr, 0);
    MainWatch::EventLoopOnce(zk);
    MainWatch::EventLoopOnce(zk);
    MainWatch::EventLoopOnce(zk);
}

TEST_FINSH