/**
 * @file ConsoleSink.hpp
 * @brief Console sink for fatal-only output to system console
 *
 * This file implements a console sink for the TFCADIR logging system.
 * It outputs only critical and fatal log messages to the system console
 * (stderr), filtering out all other log levels.
 */

#pragma once

#include <mutex>
#include <string>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace TFCADIR {
namespace Logging {

/**
 * @brief Configuration structure for ConsoleSink
 *
 * Contains all configurable parameters for the console sink.
 */
struct ConsoleSinkConfig {
  /// Minimum log level to output (default: critical)
  /// Only messages at this level or higher will be output to console
  spdlog::level::level_enum level{spdlog::level::critical};

  /// Use color output (default: true)
  /// When enabled, log messages will be colored based on severity
  bool use_color{true};
};

/**
 * @brief Console sink for fatal-only output
 *
 * This sink outputs only critical and fatal log messages to the system console.
 * It filters at spdlog::level::critical, which means only Critical and Fatal
 * messages will be output to the console where the application was launched.
 *
 * This is useful for:
 * - Alerting users to critical errors in terminal
 * - Keeping console output clean from debug/info messages
 * - Ensuring important errors are visible even when GUI is not available
 *
 * Thread-safe by default (uses std::mutex).
 *
 * @tparam Mutex Mutex type for thread safety (default: std::mutex)
 *
 * @example
 * @code
 * // Create a console sink with default configuration (critical level)
 * auto sink = std::make_shared<ConsoleSink_mt>();
 *
 * // Create a console sink with custom configuration
 * ConsoleSinkConfig config;
 * config.level = spdlog::level::critical;
 * config.use_color = true;
 * auto sink = std::make_shared<ConsoleSink_mt>(config);
 *
 * // Use with spdlog logger
 * auto logger = std::make_shared<spdlog::logger>("console_logger", sink);
 * logger->critical("Critical error occurred!");  // Will be output to console
 * logger->info("Application started");           // Will NOT be output
 * @endcode
 */
template <typename Mutex>
class ConsoleSink : public spdlog::sinks::base_sink<Mutex> {
public:
  /**
   * @brief Construct a ConsoleSink with default configuration
   *
   * Creates a console sink with:
   * - Log level: critical (only critical and fatal messages)
   * - Color output: enabled
   */
  ConsoleSink();

  /**
   * @brief Construct a ConsoleSink with custom configuration
   *
   * @param config Configuration parameters for the console sink
   */
  explicit ConsoleSink(const ConsoleSinkConfig &config);

  /**
   * @brief Destructor
   *
   * Flushes any remaining buffered output.
   */
  ~ConsoleSink() override;

  // Prevent copying
  ConsoleSink(const ConsoleSink &) = delete;
  ConsoleSink &operator=(const ConsoleSink &) = delete;

  // Allow moving
  ConsoleSink(ConsoleSink &&) noexcept = default;
  ConsoleSink &operator=(ConsoleSink &&) noexcept = default;

  /**
   * @brief Get the current configuration
   *
   * @return Copy of the current ConsoleSinkConfig
   */
  [[nodiscard]] ConsoleSinkConfig get_config() const;

  /**
   * @brief Set the configuration for this sink
   *
   * Updates the sink configuration including the log level filter.
   *
   * @param config New configuration parameters
   */
  void set_config(const ConsoleSinkConfig &config);

  /**
   * @brief Set the minimum log level
   *
   * Convenience method to set the minimum log level without
   * modifying other configuration parameters.
   *
   * @param level Minimum log level for output
   */
  void set_level(spdlog::level::level_enum level);

protected:
  /**
   * @brief Process and write a log message
   *
   * Called by spdlog for each log message. Handles:
   * - Formatting the message
   * - Writing to stderr
   *
   * @param msg Log message to process
   */
  void sink_it_(const spdlog::details::log_msg &msg) override;

  /**
   * @brief Flush the console output
   *
   * Forces all buffered data to be written to stderr.
   */
  void flush_() override;

private:
  /// Configuration parameters
  ConsoleSinkConfig m_config;
};

/// Thread-safe ConsoleSink type alias
using ConsoleSink_mt = ConsoleSink<std::mutex>;

/// Single-threaded ConsoleSink type alias (no mutex overhead)
using ConsoleSink_st = ConsoleSink<spdlog::details::null_mutex>;

/**
 * @brief Factory function to create a ConsoleSink with default configuration
 *
 * Convenience function for creating a shared pointer to a thread-safe
 * ConsoleSink with default settings (critical level, color enabled).
 *
 * @return Shared pointer to a new ConsoleSink_mt instance
 */
std::shared_ptr<ConsoleSink_mt> create_console_sink();

/**
 * @brief Factory function to create a ConsoleSink with custom configuration
 *
 * Creates a ConsoleSink with the specified configuration.
 *
 * @param config Configuration for the console sink
 * @return Shared pointer to a new ConsoleSink_mt instance
 */
std::shared_ptr<ConsoleSink_mt>
create_console_sink(const ConsoleSinkConfig &config);

} // namespace Logging
} // namespace TFCADIR
