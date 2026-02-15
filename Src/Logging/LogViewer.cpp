/**
 * @file LogViewer.cpp
 * @brief Implementation of the ImGUI-based log viewer component
 */

#include "LogViewer.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <imgui.h>

namespace TFCADIR {
namespace Logging {

//
// Constructor implementations
//

LogViewer::LogViewer(std::shared_ptr<ImGUI_sink_mt> sink)
    : sink_{std::move(sink)}, config_{}, filter_level_{spdlog::level::trace},
      auto_scroll_{true} {}

LogViewer::LogViewer(std::shared_ptr<ImGUI_sink_mt> sink,
                     const LogViewerConfig &config)
    : sink_{std::move(sink)}, config_{config},
      filter_level_{spdlog::level::trace}, auto_scroll_{true} {}

//
// Public method implementations
//

void LogViewer::render() {
  // Set initial window size
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(config_.window_width),
                                  static_cast<float>(config_.window_height)),
                           ImGuiCond_FirstUseEver);

  // Begin the log viewer window
  if (!ImGui::Begin(config_.window_title.c_str())) {
    ImGui::End();
    return;
  }

  // Render toolbar with controls
  render_toolbar();

  // Separator between toolbar and log table
  ImGui::Separator();

  // Render the log entries table
  render_log_table();

  ImGui::End();
}

void LogViewer::clear() {
  if (sink_) {
    sink_->clear();
  }
}

void LogViewer::set_filter(spdlog::level::level_enum level) {
  filter_level_ = level;
}

spdlog::level::level_enum LogViewer::get_filter() const {
  return filter_level_;
}

void LogViewer::set_search(const std::string &search) { search_text_ = search; }

std::string LogViewer::get_search() const { return search_text_; }

void LogViewer::set_auto_scroll(bool enabled) { auto_scroll_ = enabled; }

bool LogViewer::is_auto_scroll() const { return auto_scroll_; }

LogViewerConfig LogViewer::get_config() const { return config_; }

void LogViewer::set_config(const LogViewerConfig &config) { config_ = config; }

//
// Private method implementations
//

ImVec4 LogViewer::get_level_color(spdlog::level::level_enum level) const {
  // Color scheme for log levels
  switch (level) {
  case spdlog::level::trace:
    return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
  case spdlog::level::debug:
    return ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Dark Gray
  case spdlog::level::info:
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
  case spdlog::level::warn:
    return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
  case spdlog::level::err:
    return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
  case spdlog::level::critical:
    return ImVec4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
  case spdlog::level::off:
  default:
    return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Light Gray
  }
}

std::string LogViewer::format_timestamp(
    const std::chrono::system_clock::time_point &timestamp) {
  auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()) %
            1000;

  std::ostringstream oss;
  /// Use thread-safe localtime_s (Windows) or localtime_r (POSIX)
  struct tm tm_result;
#ifdef _WIN32
  /// Windows: localtime_s(&result, &input) - parameters are REVERSED!
  localtime_s(&tm_result, &time_t_val);
#else
  /// POSIX: localtime_r(&input, &result)
  localtime_r(&time_t_val, &tm_result);
#endif
  oss << std::put_time(&tm_result, "%H:%M:%S");
  oss << "." << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

bool LogViewer::matches_filter(const LogEntry &entry) const {
  // Check log level filter
  if (entry.level < filter_level_) {
    return false;
  }

  // Check search text filter (case-insensitive)
  if (!search_text_.empty()) {
    std::string search_lower = search_text_;
    std::transform(
        search_lower.begin(), search_lower.end(), search_lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::string message_lower = entry.message;
    std::transform(
        message_lower.begin(), message_lower.end(), message_lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (message_lower.find(search_lower) == std::string::npos) {
      return false;
    }
  }

  return true;
}

void LogViewer::render_toolbar() {
  // Log level filter combo box
  const char *level_names[] = {"Trace", "Debug",    "Info", "Warning",
                               "Error", "Critical", "Off"};
  int current_level = static_cast<int>(filter_level_);

  ImGui::Text("Level:");
  ImGui::SameLine();
  ImGui::PushItemWidth(100);
  if (ImGui::Combo("##LevelFilter", &current_level, level_names,
                   IM_ARRAYSIZE(level_names))) {
    filter_level_ = static_cast<spdlog::level::level_enum>(current_level);
  }
  ImGui::PopItemWidth();

  ImGui::SameLine();

  // Search input
  ImGui::Text("Search:");
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  char search_buffer[256];
  strncpy(search_buffer, search_text_.c_str(), sizeof(search_buffer) - 1);
  search_buffer[sizeof(search_buffer) - 1] = '\0';
  if (ImGui::InputText("##Search", search_buffer, sizeof(search_buffer))) {
    search_text_ = search_buffer;
  }
  ImGui::PopItemWidth();

  ImGui::SameLine();

  // Auto-scroll checkbox - detect when user explicitly changes it
  bool prev_auto_scroll = auto_scroll_;
  ImGui::Checkbox("Auto-scroll", &auto_scroll_);
  if (auto_scroll_ != prev_auto_scroll) {
    user_set_auto_scroll_ = true;
  }

  ImGui::SameLine();

  // Clear button
  if (ImGui::Button("Clear")) {
    clear();
  }

  ImGui::SameLine();

  // Copy to clipboard button
  if (ImGui::Button("Copy All")) {
    if (sink_) {
      auto entries = sink_->get_entries();
      std::ostringstream oss;
      for (const auto &entry : entries) {
        auto level_sv = spdlog::level::to_string_view(entry.level);
        oss << format_timestamp(entry.timestamp) << " ["
            << std::string(level_sv.data(), level_sv.size()) << "] "
            << entry.message;
        if (!entry.source_info.empty()) {
          oss << " (" << entry.source_info << ")";
        }
        oss << "\n";
      }
      ImGui::SetClipboardText(oss.str().c_str());
    }
  }

  // Display entry count
  if (sink_) {
    ImGui::SameLine();
    ImGui::Text("Entries: %zu", sink_->get_entry_count());
  }
}

void LogViewer::render_log_table() {
  // Get available region for the table
  ImVec2 available_size = ImGui::GetContentRegionAvail();

  // Create a child window for scrolling
  if (ImGui::BeginChild("LogEntries", available_size, false,
                        ImGuiWindowFlags_HorizontalScrollbar)) {
    // Get entries from sink
    if (sink_) {
      auto entries = sink_->get_entries();

      // Track if we need to scroll to bottom
      bool has_new_entries = false;
      if (entries.size() > last_entry_count_) {
        has_new_entries = true;
      }

      last_entry_count_ = entries.size();

      // Use table for better column alignment
      int num_columns = 0;
      if (config_.show_timestamp)
        num_columns++;
      if (config_.show_level)
        num_columns++;
      if (config_.show_thread_id)
        num_columns++;
      if (config_.show_source_info)
        num_columns++;
      num_columns++; // Message column is always shown

      if (ImGui::BeginTable("LogTable", num_columns,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_ScrollY)) {
        // Setup columns
        if (config_.show_timestamp) {
          ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed,
                                  100.0f);
        }
        if (config_.show_level) {
          ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed,
                                  70.0f);
        }
        if (config_.show_thread_id) {
          ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed,
                                  60.0f);
        }
        if (config_.show_source_info) {
          ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed,
                                  150.0f);
        }
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        // Display filtered entries
        int visible_count = 0;
        for (const auto &entry : entries) {
          // Apply filter
          if (!matches_filter(entry)) {
            continue;
          }

          // Limit visible lines for performance
          if (visible_count >= config_.max_visible_lines) {
            break;
          }
          visible_count++;

          ImGui::TableNextRow();

          // Timestamp column
          if (config_.show_timestamp) {
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(format_timestamp(entry.timestamp).c_str());
          }

          // Level column
          if (config_.show_level) {
            int level_col = config_.show_timestamp ? 1 : 0;
            ImGui::TableSetColumnIndex(level_col);
            ImVec4 color = get_level_color(entry.level);
            ImGui::TextColored(
                color, "%s", spdlog::level::to_string_view(entry.level).data());
          }

          // Thread ID column
          if (config_.show_thread_id) {
            int thread_col =
                (config_.show_timestamp ? 1 : 0) + (config_.show_level ? 1 : 0);
            ImGui::TableSetColumnIndex(thread_col);
            ImGui::TextUnformatted(entry.thread_id_str.c_str());
          }

          // Source info column
          if (config_.show_source_info) {
            int source_col = (config_.show_timestamp ? 1 : 0) +
                             (config_.show_level ? 1 : 0) +
                             (config_.show_thread_id ? 1 : 0);
            ImGui::TableSetColumnIndex(source_col);
            ImGui::TextUnformatted(entry.source_info.c_str());
          }

          // Message column
          int msg_col = num_columns - 1;
          ImGui::TableSetColumnIndex(msg_col);
          ImVec4 color = get_level_color(entry.level);
          ImGui::TextColored(color, "%s", entry.message.c_str());
        }

        // Auto-scroll to bottom if enabled and new entries arrived
        if (auto_scroll_ && has_new_entries) {
          ImGui::SetScrollHereY(1.0f);
        }

        /// Detect user scroll position and update auto-scroll state
        /// Only run when there are no new entries to avoid interfering with
        /// auto-scroll. Also respect user's explicit checkbox choice.
        if (!has_new_entries && !user_set_auto_scroll_) {
          float scroll_y = ImGui::GetScrollY();
          float scroll_max_y = ImGui::GetScrollMaxY();
          constexpr float scroll_threshold = 10.0f;

          if (scroll_y >= scroll_max_y - scroll_threshold) {
            /// User scrolled to bottom - enable auto-scroll
            auto_scroll_ = true;
          } else {
            /// User scrolled away from bottom - disable auto-scroll
            auto_scroll_ = false;
          }
        }

        /// Reset user intent flag after processing scroll detection
        /// This must be OUTSIDE the above block to be reachable when
        /// user_set_auto_scroll_ is true
        if (!has_new_entries && user_set_auto_scroll_) {
          user_set_auto_scroll_ = false;
        }

        ImGui::EndTable();
      }
    }
  }
  ImGui::EndChild();
}

//
// Factory function implementations
//

std::unique_ptr<LogViewer>
create_log_viewer(std::shared_ptr<ImGUI_sink_mt> sink) {
  return std::make_unique<LogViewer>(std::move(sink));
}

std::unique_ptr<LogViewer>
create_log_viewer(std::shared_ptr<ImGUI_sink_mt> sink,
                  const LogViewerConfig &config) {
  return std::make_unique<LogViewer>(std::move(sink), config);
}

} // namespace Logging
} // namespace TFCADIR
