//
// Created by liuping on 2020/3/2.
//
#pragma once

class ClientBase {
 public:
  ClientBase()= default;
  virtual ~ClientBase() = default;
  virtual void Reconnect() = 0;

  enum ClientState {
    kConnecting =1,
    kConnected,
    kDisConnecting,
    kDisConnected,
  };
  void SetConnected() { state_ = kConnected; }
  void SetConnecting() { state_ = kConnecting; }
  void SetDisConnected() { state_ = kDisConnected; }
  void SetDisConnecting() { state_ = kDisConnecting; }
  bool NotNeedReconnect() { return !(State() == kConnected || State() == kConnecting || State() == kDisConnecting); }
  ClientState State() { return state_; }
 private:
  ClientState state_;
};
