//
// Created by liuping on 2019/9/8.
//
#pragma once

#include <iostream>
#include <unordered_map>
#include "commlib/co/co_task.h"
#include "commlib/singleton.h"
#include "commlib/object_pool.h"
#include "commlib/timer.h"
#include "commlib/time_mgr.h"
#include "commlib/trans/trans.h"
#include "commlib/trans/trans_msg.h"

class TransMgr : public S<TransMgr> {
 public:
  /**
   * 注册事务类T
   * @tparam T 注册的类
   * @param cmd 注册的key
   * @param count 注册T事务 的数量
   * @return
   */
  template<class T, class ...Args>
  bool RegisterCmd(uint32_t cmd, int count, Args... args) {
    if (trans_map_.find(cmd) != trans_map_.end()) {
      return false;
    }
    for (int i = 0; i < count; i++) {
      trans_map_[cmd].add(std::unique_ptr<Trans>((Trans *) new T(std::forward<Args>(args)...)));
    }
    return true;
  }

  /**
   * 唤醒co_id对应的事务。如果co_id == 0则为cmd创建一个事务
   * @param cmd
   * @param co_id
   * @return
   */
  int OnMsg(const TransMsg &msg) {
    auto cmd = msg.Cmd();
    auto co_id = msg.CoId();
    if (co_id == 0) { // init
      auto &trans_pool = trans_map_[cmd];
      if (trans_pool.empty()) {
        std::cout << "trans empty\n";
        return -1;
      }
      auto trans = trans_pool.get_shared();
      auto id = co_task_.AddTack([trans](CoYield &co) { trans->DoTask(co); });
      return co_task_.ResumeOneWithMsg(id, const_cast<TransMsg *>(&msg));
    }
    if (!co_task_.CoIdExist(co_id)) {
      std::cout << "co id not exist\n";
      return -1;
    }
    return co_task_.ResumeOneWithMsg(co_id, const_cast<TransMsg *>(&msg));
  }

  /**
   * @return 定时任务是否为空
   */
  void TickTimeOutCo() {
    return co_task_.DoTimeOutTask(time_mgr::now_ms());
  }
  friend class Trans;
 private:
  int Yield(CoYield &yield, int32_t time_out_ms) {
    return yield.Yield(time_out_ms);
  }
 private:
  std::unordered_map<uint32_t, ObjectPool<Trans>> trans_map_;
  CoTask co_task_;
//  Timer timer_;
//  std::unordered_map<uint32_t, Timer::TimerId> co_id_timer_id_;
};

template<class T, uint32_t cmd, uint16_t count = 1>
class RegisterTrans : public Trans {
 public:
  RegisterTrans() : Trans() {
    reg;
  }
 private:
  static bool reg;
};

template<class T, uint32_t cmd, uint16_t count>
bool RegisterTrans<T, cmd, count>::reg = TransMgr::get().RegisterCmd<T>(cmd, count);