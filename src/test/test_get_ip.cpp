//
// Created by liuping on 2020/2/14.
//

#include "app/common/get_ip.h"
#include <iostream>

int main() {
  auto ip_list = GetLocalIP();
  for (const auto &ip : ip_list) {
    std::cout << ip <<"\n";
  }
  return 0;
}