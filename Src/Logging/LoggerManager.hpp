/**
 * @file LoggerManager.hpp
 * @brief Logger manager for initializing and managing the logging system
 *
 * This file implements a singleton logger manager that initializes all three
 * logging sinks (FileSink, ImGUI sink, ConsoleSink) and manages the logger
 * lifecycle.
 */

#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include <spdlog/spdlog.h>

#include "ConsoleSink.hpp"
#include "FileSink.hpp"
#include "ImGUI.hpp"

namespace TFCADIR {
namespace Logging {

/**
 * @brief Configuration for the logging system
 *
 * Contains all configurable parameters for all three sinks.
 */
struct LoggingConfig {
  /// File sink configuration
  FileSinkConfig file;

  /// Console sink configuration
  ConsoleSinkConfig console;

  /// ImGUI sink configuration
  ImGUIConfig imgui;

  /// Global log level (default: info)
  spdlog::level::level_enum level{spdlog::level::info};
};

/**
 * @brief Manages the logging system initialization and configuration
 *
 * This singleton class is responsible for:
 * - Initializing all three sinks (File, ImGUI, Console)
 * - Loading configuration from YAML
 * - Providing access to the ImGUI sink for the log viewer
 * - Managing logger lifecycle
 *
 * Thread-safe: Uses mutex for all operations.
 *
 * @example
 * @code
 * // Initialize the logging system
 * if (!LoggerManager::instance().initialize("config/logging.yaml")) {
 *     std::cerr << "Failed to initialize logging" << std::endl;
 *     return 1;
 * }
 *
 * // Log messages
 * spdlog::info("Application started");
 *
 * // Get ImGUI sink for log viewer
 * auto sink = LoggerManager::instance().get_imgui_sink();
 * LogViewer viewer(sink);
 *
 * // Shutdown when done
 * LoggerManager::instance().shutdown();
 * @endcode
 */
class LoggerManager {
public:
  /**
   * @brief Get the singleton instance
   *
   * @return Reference to the singleton instance
   */
  static LoggerManager &instance();

  /**
   * @brief Initialize the logging system
   *
   * Loads configuration from the specified YAML file and initializes
   * all three sinks. If the configuration file cannot be loaded,
   * default configuration is used.
   *
   * @param config_path Path to the logging configuration file
   * @return true if initialization succeeded, false otherwise
   */
  bool initialize(const std::string &config_path = "config/logging.yaml");

  /**
   * @brief Shutdown the logging system
   *
   * Flushes and releases all loggers and sinks.
   */
  void shutdown();

  /**
   * @brief Get the ImGUI sink for log viewer integration
   *
   * @return Pointer to the ImGUI sink, or nullptr if not initialized
   */
  ImGUI_sink_mt *get_imgui_sink();

  /**
   * @brief Get the logging configuration
   *
   * @return Current logging configuration
   */
  const LoggingConfig &get_config() const { return config_; }

  /**
   * @brief Set the logging configuration
   *
   * Updates the configuration. Note: This does not reinitialize
   * the sinks if the logger is already running.
   *
   * @param config New logging configuration
   */
  void set_config(const LoggingConfig &config);

  /**
   * @brief Check if the logger is initialized
   *
   * @return true if initialized, false otherwise
   */
  bool is_initialized() const { return initialized_; }

private:
  /// Constructor (private for singleton)
  LoggerManager() = default;

  /// Destructor (private for singleton)
  ~LoggerManager();

  /// Delete copy constructor and assignment operator
  LoggerManager(const LoggerManager &) = delete;
  LoggerManager &operator=(const LoggerManager &) = delete;

  /**
   * @brief Load configuration from YAML file
   *
   * @param config_path Path to the YAML configuration file
   * @return true if loading succeeded, false otherwise
   */
  bool load_config_(const std::string &config_path);

  /**
   * @brief Initialize all sinks
   *
   * Creates and configures FileSink, ConsoleSink, and ImGUI sink.
   *
   * @return true if initialization succeeded, false otherwise
   */
  bool initialize_sinks_();

  /// Logging configuration
  LoggingConfig config_;

  /// File sink
  std::shared_ptr<FileSink_mt> file_sink_;

  /// Console sink
  std::shared_ptr<ConsoleSink_mt> console_sink_;

  /// ImGUI sink
  std::shared_ptr<ImGUI_sink_mt> imgui_sink_;

  /// Main logger
  std::shared_ptr<spdlog::logger> logger_;

  /// Mutex for thread safety
  mutable std::mutex mutex_;

  /// Initialization flag
  bool initialized_{false};
};

} // namespace Logging
} // namespace TFCADIR
