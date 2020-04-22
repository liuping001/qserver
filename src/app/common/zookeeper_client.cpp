// Copyright []
#include "zookeeper_client.h"
#include <iostream>
using namespace std;


void ZooKeeperClient::DefaultWatcher(zhandle_t *zkh, int type, int state, const char *path, void *context) {
  auto client = static_cast<ZooKeeperClient*>(context);
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      client->SetConnected();  // 在接收到ZOO_CONNECTED_STATE事件后设置状态为已连接状态
      cout << "ZOO_CONNECTED_STATE\n";
    } else if (state == ZOO_CONNECTING_STATE) {
      client->SetConnecting();
      cout << "ZOO_CONNECTING_STATE\n";
    } else if (state == ZOO_EXPIRED_SESSION_STATE) {
      client->SetDisConnected();  // 在接收到ZOO_EXPIRED_SESSION_STATE事件后设置为过期状态（并关闭会话句柄）
      cout << "ZOO_EXPIRED_SESSION_STATE\n";
    }
  } else {
    client->watcher_(type, state, path);
    cout << "type:" << type << " path:" << std::string(path) <<std::endl;
  }
}

void ZooKeeperClient::DataCompletion(int rc, const char *value, int valueLen, const struct Stat *stat,
  const void *data) {
  auto co = static_cast<CoYield *>(const_cast<void *>(data));
  if (ZOK == rc) {
    std::string ret_data(value, valueLen);
    cout << "ret data:" <<ret_data << std::endl;
    co->ResumeOneWithMsg(&ret_data);
  } else {
    // ZCONNECTIONLOSS
    // ZOPERATIONTIMEOUT
    // ZNONODE
    // ZNOAUTH
    LOG_ERROR(("data completion error, ret=%d", rc));
    co->ResumeOneWithMsg(nullptr);
  }
  //
}

void ZooKeeperClient::SVDataCompletion(int rc, const struct String_vector *strings, const void *data) {
  auto co = static_cast<CoYield *>(const_cast<void *>(data));
  if (ZOK == rc) {
    std::vector<std::string> vecs;
    for (int i = 0; i < strings->count; ++i) {
      vecs.push_back(strings->data[i]);
    }
    cout << "ret data:" <<vecs.size() << std::endl;
    co->ResumeOneWithMsg(&vecs);
  } else {
    // ZCONNECTIONLOSS
    // ZOPERATIONTIMEOUT
    // ZNONODE
    // ZNOAUTH
    LOG_ERROR(("data completion error, ret=%d", rc));
    co->ResumeOneWithMsg(nullptr);
  }
  //
}

void ZooKeeperClient::SDataCompletion(int rc, const char *value, const void *data) {
  auto co = static_cast<CoYield *>(const_cast<void *>(data));
  if (ZOK == rc) {
    co->ResumeOneWithMsg(value);
    LOG_INFO(("string completion"));
  } else {
    // ZCONNECTIONLOSS
    // ZOPERATIONTIMEOUT
    // ZNONODE
    // ZNOAUTH
    LOG_ERROR(("string completion error, ret=%d", rc));
    co->ResumeOneWithMsg(nullptr);
  }
}

void ZooKeeperClient::StatCompletion(int rc, const struct Stat *stat, const void *data) {
  auto co = static_cast<CoYield *>(const_cast<void *>(data));
  if (ZOK == rc) {
    co->ResumeOneWithMsg(stat);
    LOG_INFO(("stat completion. version:%d", stat->version));
  } else {
    // ZCONNECTIONLOSS
    // ZOPERATIONTIMEOUT
    // ZNONODE
    // ZNOAUTH
    LOG_ERROR(("data completion error, ret=%d", rc));
    co->ResumeOneWithMsg(nullptr);
  }
}

void ZooKeeperClient::EventLoopOnce(zhandle_t *zkh, uint32_t time_out_ms) {
  if (zkh == nullptr) {
    return;
  }
  int fd = 0, interest = 0, events = 0;
  fd_set rfds, wfds, efds;
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  FD_ZERO(&efds);

  struct timeval tv1 = {};
  // 注册zookeeper中的事件
  zookeeper_interest(zkh, &fd, &interest, &tv1);
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
  struct timeval tv = {};
  tv.tv_sec = time_out_ms / 1000;
  tv.tv_usec = time_out_ms % 1000;
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
  return;
}

void ZooKeeperClient::Check() {
  Reconnect();
  EventLoopOnce(zkhandle_, 0);
}

ZooKeeperClient::~ZooKeeperClient() {
  if (zkhandle_) {
    zookeeper_close(zkhandle_);
  }
}

bool ZooKeeperClient::Init(const std::string &host_info, uint32_t time_out) {
  host_info_ = host_info;
  time_out_ = time_out;
  zkhandle_ =  zookeeper_init(host_info_.c_str(), DefaultWatcher, time_out_, nullptr, this, 0);
  for (int i = 0; i < 10; i++) {
    EventLoopOnce(zkhandle_, 1000);
    if (State() == kConnected) {
      return true;
    }
    if (State() == kDisConnected) {
      return false;
    }
  }
  return false;
}

void ZooKeeperClient::Reconnect() {
  if (zkhandle_ == nullptr) {  // 还未初始化
    return;
  }
  if (NotNeedReconnect()) {
    return;
  }
  if (zkhandle_) {
    zookeeper_close(zkhandle_);
  }
  zkhandle_ =  zookeeper_init(host_info_.c_str(), DefaultWatcher, time_out_, nullptr, this, 0);
}

int ZooCmd::Create(const std::string &path, const std::string &value, bool temp) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  int flag = temp ? ZOO_EPHEMERAL : 0;
  auto z_ret = zoo_acreate(client_.zkhandle_, path.c_str(), value.c_str(), value.size(),
    &ZOO_OPEN_ACL_UNSAFE, flag, ZooKeeperClient::SDataCompletion, &co_);
  if (z_ret != ZOK) {
    return -1;
  }
  auto ret = co_.Yield();
  if (ret != 0) {
    return -1;
  }
  auto data = co_.GetMsg();
  if (data == nullptr) {
    return -1;
  }
  return 0;
}

bool ZooCmd::Set(const std::string &path, const std::string &value) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  auto z_ret = zoo_aset(client_.zkhandle_, path.c_str(), value.c_str(), value.size(), -1, ZooKeeperClient::StatCompletion, &co_);
  if (z_ret != ZOK) {
    return -1;
  }
  auto ret = co_.Yield();
  if (ret != 0) {
    return false;
  }
  auto data = co_.GetMsg();
  if (data == nullptr) {
    return false;
  }
  return true;
}

std::string ZooCmd::Get(const std::string &path) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  auto z_ret = zoo_aget(client_.zkhandle_, path.c_str(), 0, ZooKeeperClient::DataCompletion, &co_);
  if (z_ret != ZOK) {
    return {};
  }
  auto ret = co_.Yield();
  if (ret == 0) {
    auto data = co_.GetMsg();
    if (data != nullptr) {
      return *(std::string *)(data);
    }
  }
  return {};
}

std::string ZooCmd::WGet(const std::string &path) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  auto z_ret = zoo_awget(client_.zkhandle_, path.c_str(), ZooKeeperClient::DefaultWatcher, &client_,
   ZooKeeperClient::DataCompletion, &co_);
  if (z_ret != ZOK) {
    return {};
  }
  auto ret = co_.Yield();
  if (ret == 0) {
    auto data = co_.GetMsg();
    if (data != nullptr) {
      return *(std::string *)(data);
    }
  }
  return {};
}

std::vector<std::string> ZooCmd::GetChildren(const std::string &path) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  auto z_ret = zoo_aget_children(client_.zkhandle_, path.c_str(), 0, ZooKeeperClient::SVDataCompletion, &co_);
  if (z_ret != ZOK) {
    return {};
  }
  auto ret = co_.Yield();
  if (ret == 0) {
    auto data = co_.GetMsg();
    if (data != nullptr) {
      return *(std::vector<std::string> *)(data);
    }
  }
  return {};
}

std::vector<std::string> ZooCmd::WGetChildRen(const std::string &path) {
  if (client_.State() != ClientBase::kConnected) {
    return {};
  }
  auto z_ret = zoo_awget_children(client_.zkhandle_, path.c_str(), ZooKeeperClient::DefaultWatcher, &client_,
    ZooKeeperClient::SVDataCompletion, &co_);
  if (z_ret != ZOK) {
    return {};
  }
  auto ret = co_.Yield();
  if (ret == 0) {
    auto data = co_.GetMsg();
    if (data != nullptr) {
      return *(std::vector<std::string> *)(data);
    }
  }
  return {};
}
