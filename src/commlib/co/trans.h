//
// Created by liuping on 2019/2/2.
//
#pragma once

#include "commlib/co/co_task.h"

class Trans {
public:
    virtual void DoTask(const CoYield &co){};
    virtual ~Trans() {}

protected:
    int Yield(const CoYield &yield, int32_t time_out_ms = 5000);
};
