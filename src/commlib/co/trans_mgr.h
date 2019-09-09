//
// Created by liuping on 2019/9/8.
//
#pragma once

#include <iostream>
#include <unordered_map>
#include "commlib/co/trans.h"
#include "commlib/co/co_task.h"
#include "commlib/singleton.h"
#include "commlib/object_pool.h"

class TransMgr : public S<TransMgr> {
public:
    template <class T>
    bool RegisterCmd(uint32_t cmd, int count) {
        if (trans_map_.find(cmd) != trans_map_.end()) {
            return false;
        }
        for (int i = 0; i < count; i++) {
            trans_map_[cmd].add(std::unique_ptr<Trans>((Trans *) new T()));
        }
        return true;
    }

    int OnCmd(uint32_t cmd, uint32_t co_id) {
        if (co_id == 0) {
            auto &trans_pool = trans_map_[cmd];
            if (trans_pool.empty()) {
                return -1;
            }
            auto trans = trans_pool.get_shared();
            auto id = co_task_.AddTack([trans](const CoYield &co) { trans->DoTask(co); });
            return co_task_.ResumeOne(id);
        }
        return co_task_.ResumeOne(co_id);
    }


private:
    std::unordered_map<int, ObjectPool<Trans>> trans_map_;
    CoTask co_task_;
};

template <class T, uint32_t cmd, uint16_t count = 1>
class RegisterTrans : public Trans {
public:
    RegisterTrans() : Trans() {
        reg;
    }
private:
    static bool reg;
};

template <class T, uint32_t cmd, uint16_t count>
bool RegisterTrans<T, cmd, count>::reg = TransMgr::get().RegisterCmd<T>(cmd, count);