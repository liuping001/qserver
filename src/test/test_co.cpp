//
// Created by liuping on 2019/9/8.
//

#include <iostream>
#include "commlib/trans/trans_mgr.h"

enum {
  CMD_TRANSA = 1,
  CMD_TRANSB,
  CMD_TRANSC
};

class TransA : public RegisterTrans<TransA, CMD_TRANSA> {
 public:
  TransA() {};
  void DoTask(const CoYield &co) final {
    std::cout << "yield before " << co.co_id_ << "\n";
    auto ret = Yield(co, 1000);
    if (ret != 0) {
      std::cout << " by time out 1 \n";
    }
    ret = Yield(co, 1000);
    if (ret != 0) {
      std::cout << " by time out 2 \n";
    }
    std::cout << "yield after " << co.co_id_ << "\n";
  }
};

class TransB : public RegisterTrans<TransB, CMD_TRANSB> {
 public:
  TransB() {};
  void DoTask(const CoYield &co) final {
    std::cout << "yield before " << co.co_id_ << "\n";
    auto ret = Yield(co);
    if (ret != 0) {
      std::cout << " by time out\n";
    }
    std::cout << "yield after " << co.co_id_ << "\n";
  }
};

class TransC : public RegisterTrans<TransC, CMD_TRANSC> {
 public:
  TransC() {};
  void DoTask(const CoYield &co) final {
    std::cout << "yield before " << co.co_id_ << "\n";
    auto ret = Yield(co);
    if (ret != 0) {
      std::cout << " by time out\n";
    }
    std::cout << "yield after " << co.co_id_ << "\n";
  }
};

struct MsgType : public TransMsg {
  uint32_t Cmd() const final {
    return cmd;
  }
  uint32_t CoId() const final {
    return co_id;
  }
  MsgType(uint32_t _cmd, uint32_t _co_id) : cmd(_cmd), co_id(_co_id) {}
  uint32_t cmd;
  uint32_t co_id;
};

int main() {
  TransMgr::get().OnMsg(MsgType{CMD_TRANSA, 0}); // co_id 1
  TransMgr::get().OnMsg(MsgType{CMD_TRANSB, 0}); // co_id 2
  TransMgr::get().OnMsg(MsgType{CMD_TRANSC, 0}); // co_id 3
  TransMgr::get().OnMsg(MsgType{CMD_TRANSB, 2});
  TransMgr::get().OnMsg(MsgType{CMD_TRANSC, 3});
  while (1) {
    auto b = TransMgr::get().TickTimeOutCo();
    if (b) {
      return 0;
    }
    time_mgr::sleep(100);
  }
}