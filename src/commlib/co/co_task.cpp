//
// Created by admin on 2018/12/19.
//
#include <vector>
#include "co_task.h"

int CoTask::ResumeAll() {
  const auto &action_co = co_pool_.GetActionCo();
  if (action_co.empty()) {
    return 0;
  }
  std::vector<int> co_id_list;
  for (const auto &co_id : action_co) {
    co_id_list.push_back(co_id.first);
  }
  for (auto co_id : co_id_list) {
    co_pool_.Resume(co_id, false);
  }
  return co_id_list.size();
}

void CoTask::Function(void *co_pool, void *co, void *co_task) {
  auto &co_task_ = *static_cast<CoTask *>(co_task);
  auto &co_ = *static_cast<Coroutine *>(co);
  CoYield co_yield(co_task_, co_.co_id);
  co_.task(co_yield);
}