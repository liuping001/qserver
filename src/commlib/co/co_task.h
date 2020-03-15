//
// Created by admin on 2018/12/19.
//
#pragma once

#include <unordered_set>
#include <functional>
#include "commlib/timer.h"
#include "commlib/time_mgr.h"
#include "coroutine.h"


//using TaskFreeCall = std::function<void (int)>;
class CoTask {
 public:
  CoTask() {}

  uint32_t AddTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return co_id;
  }
  int Yield(uint32_t co_id, time_t time_out_ms) {
    co_id_timer_id_[co_id] = timer_.AddTimer(time_mgr::now_ms(), time_out_ms,
                                                    std::bind(&CoTask::ResumeOne, this, co_id, true));
    return co_pool_.Yield(co_id);
  }
  int DoTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return ResumeOne(co_id, false);
  }

  // 通过co_id唤醒
  int ResumeOne(uint32_t co_id, bool time_out = false) {
    CancelTimer(co_id);
    return co_pool_.Resume(co_id, time_out);
  }

  int ResumeOneWithMsg(uint32_t co_id, CoMsg co_msg) {
    CancelTimer(co_id);
    return co_pool_.ResumeWithMsg(co_id, co_msg);
  }

  // co info
  Coroutine* CoInfo(uint32_t co_id) {
    return co_pool_.FindCoId(co_id);
  }

  void FreeCoroutine(uint32_t co_id) {
    co_pool_.FreeCoroutine(co_id);
  }

  bool CoIdExist(uint32_t co_id) {
    auto ret = co_pool_.FindCoId(co_id);
    return ret != nullptr;
  }
  // 返回本次resume的个数
  int ResumeAll();

  void DoTimeOutTask(time_t now_ms) {
    return timer_.DoTimeOutTask(now_ms);
  }

 private:
  static void Function(void *co_pool, void *co, void *co_task);

  void CancelTimer(uint32_t co_id) {
    auto iter_timer_id = co_id_timer_id_.find(co_id);
    if (iter_timer_id != co_id_timer_id_.end()) {
      timer_.CancelTimer(iter_timer_id->second);
      co_id_timer_id_.erase(iter_timer_id);
    }
  }

  CoPool co_pool_;

  Timer timer_;
  std::unordered_map<uint32_t, Timer::TimerId> co_id_timer_id_;
};

class CoYield {
  friend class CoTask;
  CoTask &co_task_;
 public:
  uint32_t co_id_;
  CoYield(CoTask &co_task, int32_t co_id) : co_task_(co_task), co_id_(co_id) {}

  ~CoYield() {
    co_task_.FreeCoroutine(co_id_);
  }

  int Yield(int32_t time_out_ms = 5000) {
    return co_task_.Yield(co_id_, time_out_ms);
  }

  CoMsg GetMsg() const {
    auto co = co_task_.CoInfo(co_id_);
    if (co == nullptr) {
      return nullptr;
    }
    return co->co_msg;
  }

  int ResumeOneWithMsg(CoMsg co_msg) {
    return co_task_.ResumeOneWithMsg(co_id_, co_msg);
  }
};