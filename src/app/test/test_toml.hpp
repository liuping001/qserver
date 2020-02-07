#pragma once
#include "commlib/toml_base.hpp"

namespace test_toml {

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

} // end test_toml
