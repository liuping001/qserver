//
// Created by admin on 2018/12/19.
//
#pragma once

#include <unordered_set>
#include <functional>

#include "coroutine.h"


//using TaskFreeCall = std::function<void (int)>;
class CoTask {
 public:
  CoTask() {}

  uint32_t AddTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return co_id;
  }
  int Yield(uint32_t co_id) {
    return co_pool_.Yield(co_id);
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

 private:
  static void Function(void *co_pool, void *co, void *co_task);

  CoPool co_pool_;
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

  int Yield() const {
    return co_task_.Yield(co_id_);
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