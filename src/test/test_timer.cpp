//
// Created by liuping on 2019/9/10.
//
#include <iostream>
#include "commlib/timer.h"
#include "commlib/time_mgr.h"

int main() {
  Timer timer;
  for (int i = 0; i < 5; i++) {
    auto id = timer.AddTimer(time_mgr::now_ms() + i * 1000, [i]() { std::cout << i << "s\n"; });
    if (i == 2) {
      timer.CancelTimer(id);
    }
  }

  while (timer.TimerSize() != 0) {
    timer.DoTimeOutTask(time_mgr::now_ms());
    time_mgr::sleep(10);
  }
  return 0;
}
