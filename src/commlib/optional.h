//
// Created by liuping on 2019/12/22.
//

#pragma once

template <typename T>
class optional {
 public:
  optional() = default;

  optional(const optional &) = default;
  optional& operator=(const optional &) = default;

  optional(optional &&) = default;
  optional& operator=(optional &&) = default;

  ~optional() = default;

  template <typename ...Args>
  explicit optional(Args &&...args) : _value(true, T(std::forward<Args>(args)...)) {}

  explicit operator bool() const {
    return _value.first;
  }

  T& value() {
    return _value.second;
  }

  const T& value() const {
    return _value.second;
  }

  T* operator->() {
    return &(_value.second);
  }

  const T* operator->() const {
    return &(_value.second);
  }

  T& operator*() {
    return _value.second;
  }

  const T& operator*() const {
    return _value.second;
  }

 private:
  std::pair<bool, T> _value;
};

