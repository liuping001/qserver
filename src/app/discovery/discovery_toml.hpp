#pragma once
#include "commlib/toml_base.hpp"

namespace discovery_toml {


struct Root {
  std::string router;
  std::string mq_addr;

  void FromToml(std::shared_ptr<cpptoml::base> ptr){
    router = ptr->as_table()->get("router")->as<std::string>()->get();
    mq_addr = ptr->as_table()->get("mq_addr")->as<std::string>()->get();
  }
};

} // end redissvr_toml
