//
// Created by liuping on 2020/3/15.
//

#include <string>
#include <cstdlib>

#include "test_base.hpp"

#define private public
#include "commlib/consistent_hash.hpp"

struct Random {
  Random() {
    srand (time(NULL));
  }
  // [0,max)
  int RandOne(int max) {
    return rand() % max;
  }
};

TEST_F(test_size) {
  ConsistentHash ch;
  // 第一次
  auto add_remove = [&]() {
    ch.AddNode("node1");
    ch.AddNode("node2");
    ch.RemoveNode("node2");
    ch.RemoveNode("node1");
  };
  add_remove();
  add_remove();
}

TEST_F(hash) {
  ConsistentHash ch;
  ch.AddNode("222-node-222");
  ch.AddNode("333-node-333");
  ch.AddNode("444-node-444");
  ch.AddNode("555-node-555");
  ch.AddNode("666-node-666");
  ch.AddNode("111-node-111");


  Random gen_int;
  std::map<std::string, uint32_t> count;
  for (int i =0; i <100000; i++) {
    count[ch.GetNode(gen_int.RandOne(1000000))]++;
  }
  for (auto &item : count) {
    INFO << item.first <<" " <<item.second;
  }
  count.clear();
  ch.RemoveNode("111-node-111");
  ch.RemoveNode("222-node-222");
  ch.RemoveNode("333-node-333");

  INFO <<"-------------------";
  for (int i =0; i <100000; i++) {
    count[ch.GetNode(gen_int.RandOne(1000000))]++;
  }
  for (auto &item : count) {
    INFO << item.first <<" "<< item.second;
  }
  count.clear();
}

TEST_FINSH