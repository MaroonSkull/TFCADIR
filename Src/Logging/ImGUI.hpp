/**
 * @file ImGUI.hpp
 * @brief ImGUI sink for spdlog with circular buffer for in-app log display
 *
 * This file implements a thread-safe sink for spdlog that captures log messages
 * in a circular buffer for display in an ImGUI-based log viewer.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

namespace TFCADIR {
namespace Logging {

/**
 * @brief Represents a single log entry in the circular buffer
 *
 * Contains all information needed to display a log message in the viewer,
 * including timestamp, severity level, message text, and source location.
 */
struct LogEntry {
  /// Time when the log message was created
  std::chrono::system_clock::time_point timestamp;

  /// Log severity level (trace, debug, info, warning, error, critical)
  spdlog::level::level_enum level;

  /// The formatted log message text
  std::string message;

  /// ID of the thread that generated this log message (as string for
  /// portability)
  std::string thread_id_str;

  /// Source file and line information (format: "file:line")
  std::string source_info;
};

/**
 * @brief Configuration structure for ImGUI sink
 *
 * Contains all configurable parameters for the ImGUI sink.
 */
struct ImGUIConfig {
  /// Maximum number of log entries to keep in the circular buffer (default:
  /// 10000)
  size_t max_entries{10000};

  /// Initial log level filter (default: trace, showing all messages)
  spdlog::level::level_enum initial_level{spdlog::level::trace};
};

/**
 * @brief Thread-safe circular buffer sink for ImGUI log display
 *
 * This class extends spdlog's base_sink to capture log messages in a circular
 * buffer for display in an ImGUI-based log viewer. Features include:
 * - Fixed-size circular buffer with automatic overflow handling
 * - Thread-safe access using mutex
 * - Low overhead (< 0.1ms per log call)
 * - Support for log level filtering
 * - Source location tracking
 *
 * @tparam Mutex Mutex type for thread safety (default: std::mutex)
 *
 * @example
 * @code
 * // Create an ImGUI sink with default configuration
 * auto sink = std::make_shared<ImGUI_sink_mt>();
 *
 * // Create an ImGUI sink with custom configuration
 * ImGUIConfig config;
 * config.max_entries = 5000;
 * config.initial_level = spdlog::level::debug;
 * auto sink = std::make_shared<ImGUI_sink_mt>(config);
 *
 * // Use with spdlog logger
 * auto logger = std::make_shared<spdlog::logger>("imgui_logger", sink);
 * logger->info("Application started");
 *
 * // In ImGUI render loop, get entries for display
 * auto entries = sink->get_entries();
 * for (const auto& entry : entries) {
 *     // Render entry in ImGUI
 * }
 * @endcode
 */
template <typename Mutex>
class ImGUI_sink : public spdlog::sinks::base_sink<Mutex> {
public:
  /**
   * @brief Construct an ImGUI sink with default configuration
   *
   * Creates a sink with:
   * - Buffer size: 10,000 entries
   * - Initial level: trace (all messages)
   */
  ImGUI_sink();

  /**
   * @brief Construct an ImGUI sink with custom configuration
   *
   * @param config Configuration parameters for the sink
   */
  explicit ImGUI_sink(const ImGUIConfig &config);

  /**
   * @brief Destructor
   *
   * Clears the buffer and releases all resources.
   */
  ~ImGUI_sink() override = default;

  // Prevent copying
  ImGUI_sink(const ImGUI_sink &) = delete;
  ImGUI_sink &operator=(const ImGUI_sink &) = delete;

  // Allow moving
  ImGUI_sink(ImGUI_sink &&) noexcept = default;
  ImGUI_sink &operator=(ImGUI_sink &&) noexcept = default;

  /**
   * @brief Get all log entries from the buffer
   *
   * Returns a copy of all entries currently in the buffer.
   * Thread-safe: acquires mutex lock during operation.
   *
   * @return Vector of LogEntry structures
   */
  [[nodiscard]] std::vector<LogEntry> get_entries() const;

  /**
   * @brief Get the number of entries currently in the buffer
   *
   * Thread-safe: acquires mutex lock during operation.
   *
   * @return Number of log entries in the buffer
   */
  [[nodiscard]] size_t get_entry_count() const;

  /**
   * @brief Clear all entries from the buffer
   *
   * Thread-safe: acquires mutex lock during operation.
   */
  void clear();

  /**
   * @brief Get the current configuration
   *
   * @return Copy of the current ImGUIConfig
   */
  [[nodiscard]] ImGUIConfig get_config() const;

  /**
   * @brief Check if the buffer has reached maximum capacity
   *
   * Thread-safe: acquires mutex lock during operation.
   *
   * @return true if buffer is full, false otherwise
   */
  [[nodiscard]] bool is_full() const;

protected:
  /**
   * @brief Process and store a log message
   *
   * Called by spdlog for each log message. Handles:
   * - Extracting timestamp, level, message, and source info
   * - Storing in circular buffer with overflow handling
   *
   * @param msg Log message to process
   */
  void sink_it_(const spdlog::details::log_msg &msg) override;

  /**
   * @brief Flush the sink (no-op for in-memory buffer)
   *
   * This is a no-op since the ImGUI sink stores data in memory
   * and doesn't require explicit flushing.
   */
  void flush_() override;

private:
  /// Circular buffer for storing log entries
  std::deque<LogEntry> buffer_;

  /// Maximum number of entries in the buffer
  size_t max_entries_;

  /// Configuration parameters
  ImGUIConfig config_;

  /// Mutex for protecting buffer access in public methods
  /// Note: base_sink's mutex is used for sink_it_ protection
  mutable std::mutex buffer_mutex_;
};

/// Thread-safe ImGUI sink type alias
using ImGUI_sink_mt = ImGUI_sink<std::mutex>;

/// Single-threaded ImGUI sink type alias (no mutex overhead)
using ImGUI_sink_st = ImGUI_sink<spdlog::details::null_mutex>;

/**
 * @brief Factory function to create an ImGUI sink with default configuration
 *
 * Convenience function for creating a shared pointer to a thread-safe
 * ImGUI sink.
 *
 * @return Shared pointer to a new ImGUI_sink_mt instance
 */
std::shared_ptr<ImGUI_sink_mt> create_imgui_sink();

/**
 * @brief Factory function to create an ImGUI sink with custom configuration
 *
 * Creates an ImGUI sink with the specified configuration.
 *
 * @param config Configuration for the sink
 * @return Shared pointer to a new ImGUI_sink_mt instance
 */
std::shared_ptr<ImGUI_sink_mt> create_imgui_sink(const ImGUIConfig &config);

} // namespace Logging
} // namespace TFCADIR
