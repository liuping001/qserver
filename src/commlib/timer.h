//
// Created by liuping on 2019/9/10.
//

#pragma once

#include <set>
#include <functional>
#include <vector>

class Timer {
public:
    using TimeOutTask = std::function<void ()>;
    struct TimeInfo {
        TimeOutTask task;
    };
    using TimerId = std::pair<int64_t, TimeInfo *>;
    TimerId AddTimer(int64_t ms, TimeOutTask &&task) {
        TimerId ret(ms, new TimeInfo({std::move(task)}));
        timer_set_.insert(ret);
        return ret;
    }
    void CancelTimer(const TimerId &timer_id) {
        timer_set_.erase(timer_id);
    }
    void DoTimeOutTask(int64_t now_ms) {
        auto end = timer_set_.lower_bound(TimerId(now_ms + 1, nullptr));
        if (end == timer_set_.begin()) {
            return;
        }
        std::vector<std::pair<int64_t, TimeInfo *>> time_out_set(timer_set_.begin(), end);
        timer_set_.erase(timer_set_.begin(), end);
        for (auto &item : time_out_set) {
            item.second->task();
            delete item.second;
        }
    }
    size_t TimerSize() {
        return timer_set_.size();
    }
private:
    std::set<std::pair<int64_t, TimeInfo *>> timer_set_;
};
