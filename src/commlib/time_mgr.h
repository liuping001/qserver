#pragma once

#include <chrono>
using namespace std::chrono;

namespace time_mgr {
    inline time_t now() {
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
    inline time_t now_ms() {
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
}