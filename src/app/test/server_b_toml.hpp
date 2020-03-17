/*********************************************************
*
* 文件自动生成. tool: https://github.com/liuping001/toml_cpp
*
**********************************************************/

#pragma once
#include "third/cpptoml/cpptoml.h"


#ifndef TOML_BASE_STRUCT
#define TOML_BASE_STRUCT
struct TomlBase {
  TomlBase(){}
  TomlBase(std::shared_ptr<cpptoml::base> ptr) : ptr_(ptr) {}
  std::string operator ()() const { return ptr_->as<std::string>()->get(); }
  int64_t I() const { return ptr_->as<int64_t>()->get(); }
  double D() const { return ptr_->as<double>()->get(); }
  bool B() const { return ptr_->as<bool>()->get(); }
#if defined(TOML_DATE)
  cpptoml::local_date LocalDate() const { return ptr_->as<cpptoml::local_date>()->get(); }
  cpptoml::local_time LocalTime() const { return ptr_->as<cpptoml::local_time>()->get(); }
  cpptoml::local_datetime LocalDatetime() const { return ptr_->as<cpptoml::local_datetime>()->get(); }
  cpptoml::offset_datetime OffsetDatetime() const { return ptr_->as<cpptoml::offset_datetime>()->get(); }
#endif
 private:
  std::shared_ptr<cpptoml::base> ptr_;
};
#endif

namespace server_b_toml {

struct Root {
  std::string self_id;
  std::string router;
  std::string mq_addr;

  void FromToml(std::shared_ptr<cpptoml::base> ptr){
    self_id = ptr->as_table()->get("self_id")->as<std::string>()->get();
    router = ptr->as_table()->get("router")->as<std::string>()->get();
    mq_addr = ptr->as_table()->get("mq_addr")->as<std::string>()->get();
  }
};

} // end server_b_toml
