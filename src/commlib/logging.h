//
// Created by liuping on 2019/12/13.
//

#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "spdlog/async.h"

namespace logger_mgr {

constexpr char sink_log[] = "common_log";
constexpr char sink_data[] = "data_log";

constexpr char default_fmt[] = "[%H:%M:%S %z] [%^%l%$] [thread %t] [%s:%#] %v";
constexpr uint32_t default_rotating_size = 1024*1024*1000;

struct Config {
  std::string log_file;
  spdlog::level::level_enum level;
  uint32_t rotating_size;
  uint32_t max_files;

  std::string data_file;
  std::string fmt;
  Config() {
    level = spdlog::level::debug;
    rotating_size = default_rotating_size;
    max_files = 10;
    fmt = default_fmt;
  }
};

struct ConfigAsync : public Config {
  ConfigAsync() : Config() {}
  size_t queue_size = 8192;
  size_t n_threads = 1;
};

void Init(const std::string &log_file, const std::string &data_file = "");
void Init(const Config &config);

void InitAsync(const std::string &log_file, const std::string &data_file = "");
void InitAsync(const ConfigAsync &config);

};

#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define DATA(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_data), __VA_ARGS__)
