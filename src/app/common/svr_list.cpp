//
// Created by liuping on 2020/2/15.
//

#include "svr_list.h"

#include "app/common/svr_comm_trans.h"
#include "app/proto/cmd.pb.h"
#include "app/proto/common.pb.h"
#include "commlib/logging.h"

void SvrList::AddSvr(const std::string &svr_id) {
  if (svr_set.find(svr_id) != svr_set.end()) {
    return;
  }

  auto type = GetType(svr_id);
  if (type.empty()) {
    return;
  }

  svr_by_type[type].push_back(svr_id);
  svr_set.insert(svr_id);
}

void SvrList::RemoveSvr(const std::string &svr_id) {
  if (svr_set.find(svr_id) == svr_set.end()) {
    return;
  }
  auto list = GetSvrListByType(GetType(svr_id));
  if (list == nullptr) {
    return;
  }
  for (auto iter = list->begin(); iter != list->end(); iter++) {
    if (*iter != svr_id) {
      continue;
    }
    list->erase(iter);
    break;
  }
  svr_set.erase(svr_id);
}

std::string SvrList::GetType (const std::string &svr_id) {
  auto pos = svr_id.find_first_of('-');
  if (pos == svr_id.npos) {
    return {};
  }
  return svr_id.substr(0, pos);
}

std::vector<std::string> *SvrList::GetSvrListByType(const std::string &type) {
  auto iter = svr_by_type.find(type);
  if (iter == svr_by_type.end()) {
    return nullptr;
  }
  return &iter->second;
}