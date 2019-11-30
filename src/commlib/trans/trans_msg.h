//
// Created by winter on 2019/9/20.
//

#pragma once

struct TransMsg {
  TransMsg() {}
  virtual ~TransMsg() {}
  virtual uint32_t Cmd() const = 0;
  virtual uint32_t CoId() const = 0;
  virtual const char *Data() const { return nullptr; }
  virtual uint32_t Size() const { return 0; }
};
