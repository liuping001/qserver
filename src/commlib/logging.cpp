//
// Created by lp on 2019/12/15.
//

#include "logging.h"

namespace logger_mgr {

void InitInner(const Config &config, bool async = false) {
  auto log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.log_file, config.rotating_size, config.max_files);
  log_sink->set_level(config.level);
  log_sink->set_pattern(config.fmt);

  if (async) {
    spdlog::register_logger(std::make_shared<spdlog::async_logger>(sink_log, log_sink, spdlog::thread_pool()));
  } else {
    spdlog::register_logger(std::make_shared<spdlog::logger>(sink_log, log_sink));
  }

  if (config.data_file.empty()) {
    return;
  }
  auto data_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(config.data_file, 0, 0);
  data_sink->set_level(spdlog::level::info);
  data_sink->set_pattern(config.fmt);
  std::vector<spdlog::sink_ptr> sink_list{log_sink, data_sink};
  if (async) {
    spdlog::register_logger(std::make_shared<spdlog::async_logger>(sink_data, sink_list.begin(), sink_list.end(), spdlog::thread_pool()));
  } else {
    spdlog::register_logger(std::make_shared<spdlog::logger>(sink_data, sink_list.begin(), sink_list.end()));
  }
}

void Init(const std::string &log_file, const std::string &data_file) {
  Config config;
  config.log_file = log_file;
  config.data_file = data_file;
  InitInner(config);
}

void Init(const std::string &log_file) {
  Config config;
  config.log_file = log_file;
  InitInner(config);
}

void Init(const Config &config) {
  InitInner(config);
}

void InitAsync(const std::string &log_file, const std::string &data_file) {
  ConfigAsync config;
  config.log_file = log_file;
  config.data_file = data_file;
  InitAsync(config);
}

void InitAsync(const std::string &log_file) {
  ConfigAsync config;
  config.log_file = log_file;
  InitAsync(config);
}

void InitAsync(const ConfigAsync &config) {
  spdlog::init_thread_pool(config.queue_size, config.n_threads);
  InitInner(config, true);
}

}