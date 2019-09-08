//
// Created by admin on 2018/12/19.
//
#include <vector>
#include "co_task.h"

uint32_t CoTask::AddTack(task_type task) {
    auto co_id = co_pool_.NewCoroutine(&CoTask::Function, std::move(task), this);
    return co_id;
}

int CoTask::ResumeOne(uint32_t co_id) {
    return co_pool_.Resume(co_id);
}

bool CoTask::CoIdExist(uint32_t co_id) {
    auto ret = co_pool_.FindCoId(co_id);
    return ret != nullptr;
}

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
        co_pool_.Resume(co_id);
    }
    return co_id_list.size();
}

void CoTask::Function(void *co_pool, void *co, void *co_task) {
    auto &co_pool_ = *static_cast<CoPool*>(co_pool);
    auto &co_ = *static_cast<Coroutine*>(co);
    auto co_task_ = static_cast<CoTask*>(co_task);

    CoYield co_yield(co_pool_, co_.co_id);
    co_.task(co_yield);
}