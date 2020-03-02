//
// Created by lp on 2019/12/15.
//

#include "logging.h"

namespace logger_mgr {

std::unordered_map<int, const char *> level_str = {{spdlog::level::trace, ".trace"},
                                                   {spdlog::level::debug, ".debug"},
                                                   {spdlog::level::info, ".info"},
                                                   {spdlog::level::warn, ".warn"},
                                                   {spdlog::level::err, ".error"},
                                                   {spdlog::level::critical, ".critical"},
                                                   {spdlog::level::off, ".off"}};

void InitInner(const Config &config, bool async = false) {
  std::vector<spdlog::sink_ptr> sink_list;
  for (int32_t level = config.level; level <= spdlog::level::err; level++) {
    auto append_file_name = "/" + config.log_name + level_str.at(level);
    auto log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.log_path + append_file_name, config.rotating_size, config.max_files);
    log_sink->set_level(spdlog::level::level_enum(level));
    log_sink->set_pattern(config.fmt);
    sink_list.push_back(log_sink);
  }

  if (async) {
    auto log = std::make_shared<spdlog::async_logger>(sink_log, sink_list.begin(), sink_list.end(), spdlog::thread_pool());
    log->set_level(config.level);
    spdlog::register_logger(log);
  } else {
    auto log = std::make_shared<spdlog::logger>(sink_log, sink_list.begin(), sink_list.end());
    log->set_level(config.level);
    spdlog::register_logger(log);
  }

  if (config.data_path.empty()) {
    return;
  }
  auto data_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(config.data_path + "/" + config.log_name + ".glog", 0, 0);
  data_sink->set_level(spdlog::level::info);
  data_sink->set_pattern(config.fmt);
  sink_list.push_back(data_sink);
  if (async) {
    spdlog::register_logger(std::make_shared<spdlog::async_logger>(sink_data, sink_list.begin(), sink_list.end(), spdlog::thread_pool()));
  } else {
    spdlog::register_logger(std::make_shared<spdlog::logger>(sink_data, sink_list.begin(), sink_list.end()));
  }
}

void Init(const std::string &name, const std::string &log_path, const std::string &data_path) {
  Config config;
  config.log_name = name;
  config.log_path = log_path;
  config.data_path = data_path;
  InitInner(config);
}

void Init(const std::string &name, const std::string &log_path) {
  Config config;
  config.log_name = name;
  config.log_path = log_path;
  InitInner(config);
}

void Init(const Config &config) {
  InitInner(config);
}

void InitAsync(const std::string &name, const std::string &log_path, const std::string &data_path) {
  ConfigAsync config;
  config.log_name = name;
  config.log_path = log_path;
  config.data_path = data_path;
  InitAsync(config);
}

void InitAsync(const std::string &name, const std::string &log_path) {
  ConfigAsync config;
  config.log_name = name;
  config.log_path = log_path;
  InitAsync(config);
}

void InitAsync(const ConfigAsync &config) {
  spdlog::init_thread_pool(config.queue_size, config.n_threads);
  InitInner(config, true);
}

}