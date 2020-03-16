//
// Created by liuping on 2020/3/15.
//

/**************************************************
 * 一致性哈希算法
 **************************************************/

#pragma once
#include <map>
#include <set>
#include <vector>
#include <string>

namespace consistent_hash_detail {
inline unsigned int Hash(const std::string &data) {
  std::hash<std::string> hash_code;
  return hash_code(data);
}

inline unsigned int Hash(uint64_t data) {
  std::hash<uint64_t> hash_code;
  return hash_code(data);
}
}

using Node = std::string;
class ConsistentHash {
 private:
  uint32_t replacement_count_ = 1 << 4; // 虚节点数，解决倾斜问题
  uint32_t kKeySize = 0;
  std::map<int64_t, std::string> hash_list_; // 哈希环
 public:
  ConsistentHash();
  void AddNode(const Node &node);
  void RemoveNode(const Node &node);
  const Node& GetNode(uint64_t hash_code);
};

ConsistentHash::ConsistentHash() {}

const Node & ConsistentHash::GetNode(uint64_t hash_code) {
  if (hash_list_.empty()) {
    throw std::runtime_error("hash_list empty");
  }
  int64_t real_hash_code = consistent_hash_detail::Hash(std::to_string(hash_code)) & (kKeySize - 1);
  auto iter = hash_list_.lower_bound(-real_hash_code);
  if (iter == hash_list_.end()) {
    return hash_list_.rbegin()->second;
  }
  return iter->second;
}

void ConsistentHash::AddNode(const Node &node) {
  for (uint32_t i = 0; i < replacement_count_; i++) {
    int64_t code = consistent_hash_detail::Hash(node + "_#" + std::to_string(i)) &(kKeySize - 1) ;
    hash_list_[-code] = node;
  }
}

void ConsistentHash::RemoveNode(const Node &node) {
  for (uint32_t i = 0; i < replacement_count_; i++) {
    int64_t code = consistent_hash_detail::Hash(node + "_#" + std::to_string(i)) & (kKeySize - 1);
    hash_list_.erase(-code);
  }
}


//namespace consistent_hash_detail {
//constexpr uint32_t hash_function_seed = 5381;
//inline unsigned int Hash(const void *key, int len) {
//  /* 'm' and 'r' are mixing constants generated offline.
//   They're not really 'magic', they just happen to work well.  */
//  uint32_t seed = hash_function_seed;
//  const uint32_t m = 0x5bd1e995;
//  const int r = 24;
//
//  /* Initialize the hash to a 'random' value */
//  uint32_t h = seed ^len;
//
//  /* Mix 4 bytes at a time into the hash */
//  const unsigned char *data = (const unsigned char *) key;
//
//  while (len >= 4) {
//    uint32_t k = *(uint32_t *) data;
//
//    k *= m;
//    k ^= k >> r;
//    k *= m;
//
//    h *= m;
//    h ^= k;
//
//    data += 4;
//    len -= 4;
//  }
//
//  /* Handle the last few bytes of the input array  */
//  switch (len) {
//    case 3: h ^= data[2] << 16;
//    case 2: h ^= data[1] << 8;
//    case 1: h ^= data[0];
//      h *= m;
//  };
//
//  /* Do a few final mixes of the hash to ensure the last few
//   * bytes are well-incorporated. */
//  h ^= h >> 13;
//  h *= m;
//  h ^= h >> 15;
//
//  return (unsigned int) h;
//}
//inline unsigned int Hash(const std::string &data) {
//  return Hash(data.data(), data.size());
//}
//}