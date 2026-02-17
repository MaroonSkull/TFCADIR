/**
 * @file LoggerManager.cpp
 * @brief Implementation of the LoggerManager singleton
 */

#include "LoggerManager.hpp"

#include <iostream>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace TFCADIR {
namespace Logging {

LoggerManager &LoggerManager::instance() {
  static LoggerManager instance;
  return instance;
}

bool LoggerManager::initialize(const std::string &config_path) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (initialized_) {
    std::cerr << "LoggerManager already initialized" << std::endl;
    return false;
  }

  // Load configuration
  if (!load_config_(config_path)) {
    // Use default configuration if loading fails
    std::cerr << "Failed to load logging configuration from " << config_path
              << ", using defaults" << std::endl;
  }

  // Initialize sinks
  if (!initialize_sinks_()) {
    std::cerr << "Failed to initialize logging sinks" << std::endl;
    return false;
  }

  initialized_ = true;
  return true;
}

void LoggerManager::shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!initialized_) {
    return;
  }

  // Shutdown logger
  if (logger_) {
    logger_->flush();
    logger_.reset();
  }

  // Reset sinks
  file_sink_.reset();
  console_sink_.reset();
  imgui_sink_.reset();

  // Drop all loggers from spdlog registry
  spdlog::drop_all();

  initialized_ = false;
}

ImGUI_sink_mt *LoggerManager::get_imgui_sink() {
  std::lock_guard<std::mutex> lock(mutex_);
  return imgui_sink_.get();
}

void LoggerManager::set_config(const LoggingConfig &config) {
  std::lock_guard<std::mutex> lock(mutex_);
  config_ = config;
}

bool LoggerManager::load_config_(const std::string &config_path) {
  try {
    YAML::Node config = YAML::LoadFile(config_path);

    // Load file sink configuration
    if (config["file"]) {
      YAML::Node file_config = config["file"];
      config_.file.base_filename =
          file_config["base_filename"].as<std::string>("logs/tfcadir.log");
      config_.file.max_file_size =
          file_config["max_file_size"].as<size_t>(10 * 1024 * 1024); // 10 MB
      config_.file.max_files = file_config["max_files"].as<size_t>(5);
      config_.file.flush_level = spdlog::level::from_str(
          file_config["flush_level"].as<std::string>("err"));
      config_.file.flush_interval =
          std::chrono::seconds(file_config["flush_interval"].as<int>(1));
    }

    // Load console sink configuration
    if (config["console"]) {
      YAML::Node console_config = config["console"];
      config_.console.level = spdlog::level::from_str(
          console_config["level"].as<std::string>("critical"));
      config_.console.use_color = console_config["use_color"].as<bool>(true);
    }

    // Load ImGUI sink configuration
    if (config["imgui"]) {
      YAML::Node imgui_config = config["imgui"];
      config_.imgui.max_entries = imgui_config["max_entries"].as<size_t>(10000);
      config_.imgui.initial_level = spdlog::level::from_str(
          imgui_config["initial_level"].as<std::string>("trace"));
    }

    // Load global log level
    if (config["level"]) {
      config_.level =
          spdlog::level::from_str(config["level"].as<std::string>("info"));
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to load logging configuration: " << e.what()
              << std::endl;
    return false;
  }
}

bool LoggerManager::initialize_sinks_() {
  try {
    // Create file sink
    file_sink_ = std::make_shared<FileSink_mt>(config_.file);

    // Create console sink
    console_sink_ = std::make_shared<ConsoleSink_mt>(config_.console);

    // Create ImGUI sink
    imgui_sink_ = std::make_shared<ImGUI_sink_mt>(config_.imgui);

    // Create logger with all three sinks
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(file_sink_);
    sinks.push_back(console_sink_);
    sinks.push_back(imgui_sink_);

    logger_ =
        std::make_shared<spdlog::logger>("tfcadir", sinks.begin(), sinks.end());
    logger_->set_level(config_.level);
    logger_->flush_on(spdlog::level::critical);

    // Set as default logger
    spdlog::set_default_logger(logger_);

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize logging sinks: " << e.what()
              << std::endl;
    return false;
  }
}

LoggerManager::~LoggerManager() { shutdown(); }

} // namespace Logging
} // namespace TFCADIR
