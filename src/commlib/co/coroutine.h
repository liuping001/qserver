
#pragma once
#include <ucontext.h>

#include <unordered_map>
#include <list>
#include <functional>
class CoYield;

using task_type = std::function<void (const CoYield &)>;
const int MAX_COROUTINE_STACK = 1024*128;

struct Coroutine {
    ucontext_t context;
    uint32_t co_id = 0;
    task_type task;
    bool resume_by_time_out_ = false;
    char stack[MAX_COROUTINE_STACK] = {}; // 私有栈
};

using ActionCo = std::unordered_map<uint32_t, Coroutine*>;
class CoPool {
    using func_hander = void (void *, void *, void *);
 public:
    int NewCoroutine(func_hander func, task_type task, void *arg);
    int Yield(uint32_t co_id);
    int Resume(uint32_t co_id, bool time_out = false);
    void FreeCoroutine(uint32_t co_id);
    const ActionCo & GetActionCo();
    Coroutine *FindCoId(uint32_t co_id);
private:
    uint32_t GetCoId();
    ucontext_t main;
    std::list<Coroutine *> free_coroutine;
    ActionCo action_coroutine;
    uint32_t inc_co_id = 0;
};
