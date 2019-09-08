// 单线程进程app基类
#pragma once

#include "time_mgr.h"

class App {
public:
    virtual int OnInit() { return 0; }
    // 如果做了实际的工作就返回true, 空转返回false
    virtual bool OnTick() { return false; }

public:
    int Init() {
        auto ret = OnInit();
        if (ret != 0) {
            exit(ret);
        }
    }

    void Run() {
        if (!OnTick()) {
            idle_times_++;
        } else {
            idle_times_ = 0;
        }

        // 如果空转次数大于100次，那就睡眠1ms
        if (idle_times_ > 100) {
            idle_times_ = 0;
            time_mgr::sleep(1);
        }
    }

private:
    int idle_times_ = 0;
};
