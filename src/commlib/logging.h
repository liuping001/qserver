//
// Created by mico on 2019/12/13.
//

#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"


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

void Init(const Config &config) {
  auto log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.log_file, config.rotating_size, config.max_files);
  log_sink->set_level(config.level);
  log_sink->set_pattern(config.fmt);
  auto log_logger = std::make_shared<spdlog::logger>(sink_log, log_sink);
  spdlog::register_logger(log_logger);

  if (config.data_file.empty()) {
    return;
  }
  auto data_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(config.data_file, 0, 0);
  data_sink->set_level(spdlog::level::info);
  data_sink->set_pattern(config.fmt);
  std::vector<spdlog::sink_ptr> sink_list{log_sink, data_sink};
  auto data_logger = std::make_shared<spdlog::logger>(sink_data, sink_list.begin(), sink_list.end());
  spdlog::register_logger(data_logger);
}

void Init(const std::string &log_file, const std::string &data_file = "") {
  Config config;
  config.log_file = log_file;
  config.data_file = data_file;
  Init(config);
}

};

#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::get(logger_mgr::sink_log), __VA_ARGS__)
#define DATA(...) SPDLOG_LOGGER_INFO(spdlog::get(logger_mgr::sink_data), __VA_ARGS__)
