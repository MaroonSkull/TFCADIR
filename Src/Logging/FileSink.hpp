/**
 * @file FileSink.hpp
 * @brief File sink for spdlog with rotation and configurable flush policies
 *
 * This file implements a rotating file sink for the TFCADIR logging system.
 * It wraps spdlog's rotating_file_sink_mt with configurable flush policies
 * and periodic flush support.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace TFCADIR {
namespace Logging {

/**
 * @brief Configuration structure for FileSink
 *
 * Contains all configurable parameters for the rotating file sink.
 */
struct FileSinkConfig {
  /// Base filename for log files (default: "logs/tfcadir.log")
  std::string base_filename{"logs/tfcadir.log"};

  /// Maximum size of each log file in bytes (default: 10MB)
  size_t max_file_size{10 * 1024 * 1024};

  /// Maximum number of rotated files to keep (default: 5)
  size_t max_files{5};

  /// Flush interval in seconds (default: 1)
  /// Used for periodic flush to ensure crash safety
  std::chrono::seconds flush_interval{1};

  /// Log level that triggers immediate flush (default: err)
  spdlog::level::level_enum flush_level{spdlog::level::err};
};

/**
 * @brief Rotating file sink with configurable flush policies
 *
 * This class extends spdlog's base_sink to provide a rotating file logger
 * with the following features:
 * - Size-based log rotation
 * - Configurable maximum file size and number of rotated files
 * - Immediate flush for Error and Critical log levels
 * - Periodic flush support for crash safety
 *
 * Thread-safe by default (uses std::mutex).
 *
 * @tparam Mutex Mutex type for thread safety (default: std::mutex)
 *
 * @example
 * @code
 * // Create a file sink with default configuration
 * auto sink = std::make_shared<FileSink_mt>();
 *
 * // Create a file sink with custom configuration
 * FileSinkConfig config;
 * config.base_filename = "logs/myapp.log";
 * config.max_file_size = 20 * 1024 * 1024;  // 20MB
 * config.max_files = 10;
 * config.flush_interval = std::chrono::seconds(5);
 * auto sink = std::make_shared<FileSink_mt>(config);
 *
 * // Use with spdlog logger
 * auto logger = std::make_shared<spdlog::logger>("file_logger", sink);
 * logger->info("Application started");
 * @endcode
 */
template <typename Mutex>
class FileSink : public spdlog::sinks::base_sink<Mutex> {
public:
  /**
   * @brief Construct a FileSink with default configuration
   *
   * Creates a rotating file sink with:
   * - Log file: logs/tfcadir.log
   * - Max file size: 10MB
   * - Max rotated files: 5
   * - Flush level: err
   * - Flush interval: 1 second
   *
   * @throws spdlog::spdlog_ex if the log file cannot be created
   */
  FileSink();

  /**
   * @brief Construct a FileSink with custom configuration
   *
   * @param config Configuration parameters for the file sink
   * @throws spdlog::spdlog_ex if the log file cannot be created
   */
  explicit FileSink(const FileSinkConfig &config);

  /**
   * @brief Construct a FileSink with individual parameters
   *
   * @param base_filename Base path for log files
   * @param max_file_size Maximum size per file in bytes
   * @param max_files Maximum number of rotated files
   * @throws spdlog::spdlog_ex if the log file cannot be created
   */
  FileSink(const std::string &base_filename,
           size_t max_file_size = 10 * 1024 * 1024, size_t max_files = 5);

  /**
   * @brief Destructor
   *
   * Flushes any remaining log messages and closes the file.
   */
  ~FileSink() override;

  // Prevent copying
  FileSink(const FileSink &) = delete;
  FileSink &operator=(const FileSink &) = delete;

  // Allow moving
  FileSink(FileSink &&) noexcept = default;
  FileSink &operator=(FileSink &&) noexcept = default;

  /**
   * @brief Manually trigger log rotation
   *
   * Forces rotation of the current log file regardless of its size.
   * The current file is renamed and a new file is created.
   */
  void rotate();

  /**
   * @brief Get the current log file size
   *
   * @return Current size of the active log file in bytes
   */
  [[nodiscard]] size_t get_current_file_size() const;

  /**
   * @brief Get the total size of all log files
   *
   * Calculates the combined size of the active log file and all
   * rotated files.
   *
   * @return Total size in bytes
   */
  [[nodiscard]] size_t get_total_size() const;

  /**
   * @brief Get the current configuration
   *
   * @return Copy of the current FileSinkConfig
   */
  [[nodiscard]] FileSinkConfig get_config() const;

  /**
   * @brief Set the flush level
   *
   * Log messages at this level or higher will trigger an immediate flush.
   *
   * @param level Log level for immediate flush
   */
  void set_flush_level(spdlog::level::level_enum level);

  /**
   * @brief Get the flush level
   *
   * @return Current flush level
   */
  [[nodiscard]] spdlog::level::level_enum get_flush_level() const;

protected:
  /**
   * @brief Process and write a log message
   *
   * Called by spdlog for each log message. Handles:
   * - Formatting the message
   * - Writing to the file
   * - Checking for rotation
   * - Immediate flush for error/critical messages
   *
   * @param msg Log message to process
   */
  void sink_it_(const spdlog::details::log_msg &msg) override;

  /**
   * @brief Flush the log file buffer
   *
   * Forces all buffered data to be written to disk.
   */
  void flush_() override;

private:
  /**
   * @brief Initialize the file sink
   *
   * Creates the log directory if needed and opens the log file.
   *
   * @throws spdlog::spdlog_ex if initialization fails
   */
  void initialize_();

  /**
   * @brief Perform log rotation
   *
   * Renames existing files and creates a new log file.
   *
   * @throws std::filesystem::filesystem_error if file operations fail
   */
  void rotate_();

  /**
   * @brief Create the log directory if it doesn't exist
   *
   * Extracts the directory path from base_filename and creates it.
   *
   * @return true if directory exists or was created, false on error
   */
  bool ensure_log_directory_exists_();

  /// Configuration parameters
  FileSinkConfig m_config;

  /// Underlying rotating file sink from spdlog
  std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_rotating_sink;

  /// Current file size (tracked for get_current_file_size)
  size_t m_current_size{0};
};

/// Thread-safe FileSink type alias
using FileSink_mt = FileSink<std::mutex>;

/// Single-threaded FileSink type alias (no mutex overhead)
using FileSink_st = FileSink<spdlog::details::null_mutex>;

/**
 * @brief Factory function to create a FileSink with default configuration
 *
 * Convenience function for creating a shared pointer to a thread-safe
 * FileSink. Also sets up periodic flush based on the configuration.
 *
 * @return Shared pointer to a new FileSink_mt instance
 * @throws spdlog::spdlog_ex if the log file cannot be created
 */
std::shared_ptr<FileSink_mt> create_file_sink();

/**
 * @brief Factory function to create a FileSink with custom configuration
 *
 * Creates a FileSink with the specified configuration and sets up
 * periodic flush based on the flush_interval setting.
 *
 * @param config Configuration for the file sink
 * @return Shared pointer to a new FileSink_mt instance
 * @throws spdlog::spdlog_ex if the log file cannot be created
 */
std::shared_ptr<FileSink_mt> create_file_sink(const FileSinkConfig &config);

} // namespace Logging
} // namespace TFCADIR
