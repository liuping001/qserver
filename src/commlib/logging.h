//
// Created by liuping on 2019/12/13.
//

#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "spdlog/async.h"
#include "spdlog/fmt/ostr.h"

namespace logger_mgr {

constexpr char sink_log[] = "common_log";
constexpr char sink_data[] = "data_log";

constexpr char default_fmt[] = "[%Y-%m-%d %H:%M:%S %f] [%^%l%$] [thread %t] [%s:%#] %v";
constexpr uint32_t default_rotating_size = 1024*1024*1000;
constexpr char unknown_name[] = "unknown";

struct Config {
  Config() {}
  std::string log_path;
  std::string data_path;
  std::string log_name = unknown_name;
  spdlog::level::level_enum level = spdlog::level::debug;
  uint32_t rotating_size = default_rotating_size;
  uint32_t max_files = 10;
  std::string fmt = default_fmt;
};

struct ConfigAsync : public Config {
  ConfigAsync() : Config() {}
  size_t queue_size = 8192;
  size_t n_threads = 1;
};

void Init(const std::string &name, const std::string &log_path, const std::string &data_path);
void Init(const std::string &name, const std::string &log_path); // no data log
void Init(const Config &config);

void InitAsync(const std::string &name, const std::string &log_path, const std::string &data_path);
void InitAsync(const std::string &name, const std::string &log_path); // no data log
void InitAsync(const ConfigAsync &config);

};

#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define WARNING(...) SPDLOG_LOGGER_WARN(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define GLOG(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_data), __VA_ARGS__)
#define FATAL(...) SPDLOG_LOGGER_ERROR(spdlog::get(logger_mgr::sink_log), __VA_ARGS__);exit(-1);