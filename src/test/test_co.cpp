//
// Created by liuping on 2019/9/8.
//

#include <iostream>
#include "commlib/co/trans_mgr.h"

class TransA : public Trans {
public:
    void DoTask(const CoYield &co) final {
        std::cout << "yield before\n";
        co.Yield();
        std::cout << "yield after\n";
    }
};

class TransB : public Trans {
public:
    void DoTask(const CoYield &co) final {
        std::cout << "yield before\n";
        co.Yield();
        std::cout << "yield after\n";
    }
};

class TransC : public Trans {
public:
    void DoTask(const CoYield &co) final {
        std::cout << "yield before\n";
        co.Yield();
        std::cout << "yield after\n";
    }
};


int main() {
    TransMgr::get().RegisterCmd<TransA>(1, 1);
    TransMgr::get().RegisterCmd<TransB>(2, 1);
    TransMgr::get().RegisterCmd<TransC>(3, 1);

    TransMgr::get().OnCmd(1, 0);
    TransMgr::get().OnCmd(1, 1);
}