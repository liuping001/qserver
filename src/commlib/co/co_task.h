//
// Created by admin on 2018/12/19.
//
#pragma once

#include <unordered_set>
#include <functional>

#include "commlib/co/coroutine.h"

class CoYield {
  friend class CoTask;
  CoPool &co_pool_;
 public:
  uint32_t co_id_;
  CoYield(CoPool &co_pool, int32_t co_id) : co_pool_(co_pool), co_id_(co_id) {}

  ~CoYield() {
    co_pool_.FreeCoroutine(co_id_);
  }
  int Yield() const {
    return co_pool_.Yield(co_id_);
  }
  CoMsg GetMsg() const {
    auto co = co_pool_.FindCoId(co_id_);
    if (co == nullptr) {
      return nullptr;
    }
    return co->co_msg;
  }
};

//using TaskFreeCall = std::function<void (int)>;
class CoTask {
 public:
  CoTask() {}

  uint32_t AddTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return co_id;
  }

  int DoTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return co_pool_.Resume(co_id, false);
  }

  // 通过co_id唤醒
  int ResumeOne(uint32_t co_id, bool time_out = false) {
    return co_pool_.Resume(co_id, time_out);
  }

  int ResumeOneWithMsg(uint32_t co_id, CoMsg co_msg) {
    return co_pool_.ResumeWithMsg(co_id, co_msg);
  }

  // 通过co_id唤醒
  bool CoIdExist(uint32_t co_id) {
    auto ret = co_pool_.FindCoId(co_id);
    return ret != nullptr;
  }

  // 返回本次resume的个数
  int ResumeAll();

 private:
  static void Function(void *co_pool, void *co, void *co_task);

  CoPool co_pool_;
};
