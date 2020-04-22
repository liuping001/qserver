//
// Created by liuping on 2020/2/15.
//
#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include "commlib/singleton.h"


struct SvrList {
  std::unordered_map<std::string /*type*/, std::vector<std::string> /*svr_list*/> svr_by_type;
  std::unordered_set<std::string> svr_set;

  void Clear () {
    svr_by_type.clear();
    svr_set.clear();
  }

  void AddSvr(const std::string &svr_id);

  void RemoveSvr(const std::string &svr_id);

  std::string GetType (const std::string &svr_id);

  std::vector<std::string> *GetSvrListByType(const std::string &type);
};

using svr_list = S<SvrList>;
