
#pragma once
#include <ucontext.h>

#include <unordered_map>
#include <list>
#include <functional>
class CoYield;

using task_type = std::function<void(CoYield &)>;
const int MAX_COROUTINE_STACK = 1024 * 1024; // 私有栈大小，malloc出来，没有真正访问内存时，并不会进行内存映射

using CoMsg =const void *;
struct Coroutine {
  Coroutine() {
    stack = (char *)malloc(MAX_COROUTINE_STACK);
  }
  ~Coroutine() {
    if (stack) {
      free(stack);
    }
  }
  ucontext_t context;
  uint32_t co_id = 0;
  task_type task;
  CoMsg co_msg;
  bool resume_by_time_out = false;
  char *stack = nullptr; //私有栈
};

using ActionCo = std::unordered_map<uint32_t, Coroutine *>;
class CoPool {
  using func_hander = void(void *, void *, void *);
 public:
  int NewCoroutine(func_hander func, task_type task, void *arg);
  int Yield(uint32_t co_id);
  int Resume(uint32_t co_id, bool time_out);
  int ResumeWithMsg(uint32_t co_id, CoMsg co_msg);
  void FreeCoroutine(uint32_t co_id);
  const ActionCo &GetActionCo();
  Coroutine *FindCoId(uint32_t co_id);
 private:
  uint32_t GetCoId();
  ucontext_t main;
  std::list<Coroutine *> free_coroutine;
  ActionCo action_coroutine;
  uint32_t inc_co_id = 0;
};
