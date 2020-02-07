#pragma once
#include "commlib/toml_base.hpp"

namespace redissvr_toml {

struct Redis {
  int64_t port;
  std::string ip;

  void FromToml(std::shared_ptr<cpptoml::base> ptr){
    port = ptr->as_table()->get("port")->as<int64_t>()->get();
    ip = ptr->as_table()->get("ip")->as<std::string>()->get();
  }
};

struct Root {
  Redis redis; 
  std::string self_id;
  std::string router;
  std::string mq_addr;

  void FromToml(std::shared_ptr<cpptoml::base> ptr){
    redis.FromToml(ptr->as_table()->get("redis"));
    self_id = ptr->as_table()->get("self_id")->as<std::string>()->get();
    router = ptr->as_table()->get("router")->as<std::string>()->get();
    mq_addr = ptr->as_table()->get("mq_addr")->as<std::string>()->get();
  }
};

} // end redissvr_toml
