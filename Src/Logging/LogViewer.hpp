/**
 * @file LogViewer.hpp
 * @brief ImGUI-based log viewer component for displaying log messages
 *
 * This file implements a log viewer component that displays log messages
 * captured by ImGUI_sink in an ImGUI window with filtering, search, and
 * auto-scroll features.
 */

#pragma once

#include <memory>
#include <string>

#include "ImGUI.hpp"

// Forward declaration to avoid including imgui.h in header
struct ImVec4;

namespace TFCADIR {
namespace Logging {

/**
 * @brief Configuration for the LogViewer appearance
 */
struct LogViewerConfig {
  /// Window title (default: "Log")
  std::string window_title{"Log"};

  /// Default window width (default: 800)
  int window_width{800};

  /// Default window height (default: 400)
  int window_height{400};

  /// Show timestamp column (default: true)
  bool show_timestamp{true};

  /// Show log level column (default: true)
  bool show_level{true};

  /// Show thread ID column (default: false)
  bool show_thread_id{false};

  /// Show source info column (default: true)
  bool show_source_info{true};

  /// Maximum visible log lines (default: 1000)
  int max_visible_lines{1000};
};

/**
 * @brief ImGUI-based log viewer component
 *
 * This class provides a complete log viewer implementation for displaying
 * log messages in an ImGUI window. Features include:
 * - Log level filtering (Trace, Debug, Info, Warning, Error, Critical)
 * - Text search functionality
 * - Auto-scroll toggle
 * - Color-coded log levels
 * - Copy to clipboard
 * - Clear log buffer
 *
 * @example
 * @code
 * // Create a log viewer with an ImGUI sink
 * auto sink = std::make_shared<ImGUI_sink_mt>();
 * LogViewer viewer(sink);
 *
 * // In ImGUI render loop
 * viewer.render();
 *
 * // Change filter level
 * viewer.set_filter(spdlog::level::warn);
 *
 * // Search for text
 * viewer.set_search("error");
 * @endcode
 */
class LogViewer {
public:
  /**
   * @brief Construct a LogViewer with an ImGUI sink
   *
   * @param sink Shared pointer to the ImGUI sink to display logs from
   */
  explicit LogViewer(std::shared_ptr<ImGUI_sink_mt> sink);

  /**
   * @brief Construct a LogViewer with custom configuration
   *
   * @param sink Shared pointer to the ImGUI sink to display logs from
   * @param config Configuration for the viewer appearance
   */
  LogViewer(std::shared_ptr<ImGUI_sink_mt> sink, const LogViewerConfig &config);

  /**
   * @brief Destructor
   */
  ~LogViewer() = default;

  // Prevent copying
  LogViewer(const LogViewer &) = delete;
  LogViewer &operator=(const LogViewer &) = delete;

  // Allow moving
  LogViewer(LogViewer &&) noexcept = default;
  LogViewer &operator=(LogViewer &&) noexcept = default;

  /**
   * @brief Render the log viewer window
   *
   * This method should be called every frame in the ImGUI render loop.
   * It displays the log viewer window with all controls and log entries.
   */
  void render();

  /**
   * @brief Clear the log buffer
   *
   * Clears all log entries from the underlying sink.
   */
  void clear();

  /**
   * @brief Set the log level filter
   *
   * Only log messages at or above this level will be displayed.
   *
   * @param level Minimum log level to display
   */
  void set_filter(spdlog::level::level_enum level);

  /**
   * @brief Get the current log level filter
   *
   * @return Current minimum log level
   */
  [[nodiscard]] spdlog::level::level_enum get_filter() const;

  /**
   * @brief Set the search text
   *
   * Only log messages containing this text will be displayed.
   * Empty string shows all messages.
   *
   * @param search Text to search for (case-insensitive)
   */
  void set_search(const std::string &search);

  /**
   * @brief Get the current search text
   *
   * @return Current search text
   */
  [[nodiscard]] std::string get_search() const;

  /**
   * @brief Toggle auto-scroll
   *
   * When enabled, the viewer automatically scrolls to show new log entries.
   *
   * @param enabled true to enable auto-scroll, false to disable
   */
  void set_auto_scroll(bool enabled);

  /**
   * @brief Check if auto-scroll is enabled
   *
   * @return true if auto-scroll is enabled, false otherwise
   */
  [[nodiscard]] bool is_auto_scroll() const;

  /**
   * @brief Get the current configuration
   *
   * @return Copy of the current LogViewerConfig
   */
  [[nodiscard]] LogViewerConfig get_config() const;

  /**
   * @brief Set the configuration
   *
   * @param config New configuration for the viewer
   */
  void set_config(const LogViewerConfig &config);

private:
  /**
   * @brief Get the color for a log level
   *
   * Returns the ImGUI color to use for displaying a log message
   * of the given level.
   *
   * @param level Log level to get color for
   * @return ImVec4 Color for the log level
   */
  [[nodiscard]] ImVec4 get_level_color(spdlog::level::level_enum level) const;

  /**
   * @brief Format timestamp for display
   *
   * Converts a timestamp to a human-readable string.
   *
   * @param timestamp Timestamp to format
   * @return Formatted timestamp string
   */
  [[nodiscard]] static std::string
  format_timestamp(const std::chrono::system_clock::time_point &timestamp);

  /**
   * @brief Check if a log entry matches the current filter
   *
   * @param entry Log entry to check
   * @return true if the entry should be displayed, false otherwise
   */
  [[nodiscard]] bool matches_filter(const LogEntry &entry) const;

  /**
   * @brief Render the toolbar with controls
   */
  void render_toolbar();

  /**
   * @brief Render the log entries table
   */
  void render_log_table();

  /// ImGUI sink to display logs from
  std::shared_ptr<ImGUI_sink_mt> sink_;

  /// Configuration for the viewer
  LogViewerConfig config_;

  /// Current log level filter
  spdlog::level::level_enum filter_level_;

  /// Current search text
  std::string search_text_;

  /// Auto-scroll enabled flag
  bool auto_scroll_;

  /// Track if user explicitly set auto-scroll via checkbox
  bool user_set_auto_scroll_{false};

  /// Last entry count for auto-scroll detection
  size_t last_entry_count_{0};
};

/**
 * @brief Factory function to create a LogViewer
 *
 * Convenience function for creating a LogViewer with default configuration.
 *
 * @param sink Shared pointer to the ImGUI sink to display logs from
 * @return Unique pointer to a new LogViewer instance
 */
std::unique_ptr<LogViewer>
create_log_viewer(std::shared_ptr<ImGUI_sink_mt> sink);

/**
 * @brief Factory function to create a LogViewer with custom configuration
 *
 * @param sink Shared pointer to the ImGUI sink to display logs from
 * @param config Configuration for the viewer
 * @return Unique pointer to a new LogViewer instance
 */
std::unique_ptr<LogViewer>
create_log_viewer(std::shared_ptr<ImGUI_sink_mt> sink,
                  const LogViewerConfig &config);

} // namespace Logging
} // namespace TFCADIR
