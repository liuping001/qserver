//
// Created by lp on 2019/9/20.
//

#include "commlib/co/trans.h"
#include "commlib/co/trans_mgr.h"

int Trans::Yield(const CoYield &yield, int32_t time_out_ms) {
    TransMgr::get().Yield(yield, time_out_ms);
}