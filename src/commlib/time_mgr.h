#pragma once

#include <chrono>
#include <thread>
using namespace std::chrono;

namespace time_mgr {
inline time_t now() {
  return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}
inline time_t now_ms() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
inline void sleep(uint32_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
}