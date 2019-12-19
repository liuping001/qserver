//
// Created by mico on 2019/12/13.
//


#include <thread>
#include "commlib/logging.h"

int main() {
  logger_mgr::ConfigAsync config;
  config.log_name = "svr";
  config.log_path = "log";
  config.data_path = "log";
  config.level = spdlog::level::trace;
  logger_mgr::InitAsync(config);
  spdlog::get(logger_mgr::sink_log)->debug("show something");
  DEBUG("{}{}", "aaa", "bbb");
  INFO("{}{}", "aaa", "bbb");
  WARNING("{}{}", "aaa", "bbb");
  ERROR("{}{}", "aaa", "bbb");
  GLOG("glog{}{}", "aaa", "bbb");

//  std::thread a([](){
//    for (int i = 0; i < 10000; i ++) {
//      INFO("test log ===========================");
//      DATA("test data ----------------------------");
//      DATA("{1} {0} {2}",std::string("heelo"), int(21), float(3.14));
//    }
//  });
//  std::thread b([](){
//    for (int i = 0; i < 10000; i ++) {
//      INFO("test log +++++++++++++++++++++++++++");
//      DATA("test data ____________________________");
//      DATA("{} {} {}",std::string("heelo"), int(21), float(3.14));
//    }
//  });
//  a.join();
//  b.join();
}