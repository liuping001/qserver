//
// Created by liuping on 2020/2/14.
//
#pragma once
#include <vector>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


std::vector<std::string> GetLocalIP() {
  std::vector<std::string> ret;
  int sockfd;
  struct ifconf ifconf;
  char buf[10240];
  //初始化ifconf
  ifconf.ifc_len = 10240;
  ifconf.ifc_buf = buf;

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
    return ret;
  }
  ioctl(sockfd, SIOCGIFCONF, &ifconf);    //获取所有接口信息
  close(sockfd);
  //接下来一个一个的获取IP地址
  auto ifreq = (struct ifreq*)buf;
  for(int i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
  {
    auto ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
    //排除127.0.0.1，继续下一个
    if(strcmp(ip,"127.0.0.1")==0)
    {
      ifreq++;
      continue;
    }
    ret.push_back(ip);
  }

  return ret;
}
