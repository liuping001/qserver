//
// Created by mico on 2019/12/13.
//


#include <thread>
#include "commlib/logging.h"

int main() {
  logger_mgr::Init("log/svr.log", "log/data.log");

  std::thread a([](){
    for (int i = 0; i < 10000; i ++) {
      INFO("test log ===========================");
      DATA("test data ----------------------------");
      DATA("{1} {0} {2}",std::string("heelo"), int(21), float(3.14));
    }
  });
  std::thread b([](){
    for (int i = 0; i < 10000; i ++) {
      INFO("test log +++++++++++++++++++++++++++");
      DATA("test data ____________________________");
      DATA("{} {} {}",std::string("heelo"), int(21), float(3.14));
    }
  });
  a.join();
  b.join();
}