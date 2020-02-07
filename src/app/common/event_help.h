//
// Created by liuping on 2020/1/26.
//

#pragma once
#include <functional>
#include <event.h>
struct EventInfo {
  struct event ev;
  struct timeval tv;
  std::function<void ()> cb;
  bool repeat = false;

  static void CB(int fd, short event, void *argc) {
    EventInfo *info = static_cast<EventInfo*>(argc);
    info->cb();
    if (info->repeat) {
      event_add(&info->ev, &info->tv);
    }
  }
};

static void EvAddTimer(struct event_base * evbase, uint32_t ms, std::function<void ()> cb, bool repeat) {
  auto info = new EventInfo();
  auto sec = ms / 1000;
  auto usec = ms % 1000;
  info->tv.tv_sec = sec;
  info->tv.tv_usec = usec * 1000;
  info->cb = std::move(cb);
  info->repeat = repeat;

  event_assign(&info->ev, evbase, -1, 0, EventInfo::CB, info);
  event_add(&info->ev, &info->tv);
}