//
// Created by liuping on 2019/9/8.
//

#include <iostream>
#include "commlib/co/trans_mgr.h"

enum {
    CMD_TRANSA = 1,
    CMD_TRANSB,
    CMD_TRANSC
};

class TransA : public RegisterTrans<TransA, CMD_TRANSA> {
public:
    TransA() {};
    void DoTask(const CoYield &co) final {
        std::cout << "yield before " << co.co_id_ <<"\n";
        co.Yield();
        std::cout << "yield after " << co.co_id_ <<"\n";
    }
};

class TransB : public RegisterTrans<TransB, CMD_TRANSB> {
public:
    TransB () {};
    void DoTask(const CoYield &co) final {
        std::cout << "yield before " << co.co_id_ <<"\n";
        co.Yield();
        std::cout << "yield after " << co.co_id_ <<"\n";
    }
};

class TransC : public RegisterTrans<TransC, CMD_TRANSC> {
public:
    TransC() {};
    void DoTask(const CoYield &co) final {
        std::cout << "yield before " << co.co_id_ <<"\n";
        co.Yield();
        std::cout << "yield after " << co.co_id_ <<"\n";
    }
};

int main() {
    TransMgr::get().OnCmd(CMD_TRANSA, 0); // co_id 1
    TransMgr::get().OnCmd(CMD_TRANSA, 0); // co_id 2
    TransMgr::get().OnCmd(CMD_TRANSB, 0); // co_id 3
    TransMgr::get().OnCmd(CMD_TRANSB, 1);
    TransMgr::get().OnCmd(CMD_TRANSC, 2);
    TransMgr::get().OnCmd(CMD_TRANSC, 3);
}