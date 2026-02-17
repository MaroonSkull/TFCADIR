/**
 * @file ImGUI.cpp
 * @brief Implementation of the ImGUI sink for TFCADIR logging system
 */

#include "ImGUI.hpp"

#include <algorithm>
#include <sstream>
#include <string_view>

namespace TFCADIR {
namespace Logging {

//
// Constructor implementations
//

template <typename Mutex>
ImGUI_sink<Mutex>::ImGUI_sink() : max_entries_{10000}, config_{} {
  // deque doesn't have reserve(), but we set max_entries for overflow control
}

template <typename Mutex>
ImGUI_sink<Mutex>::ImGUI_sink(const ImGUIConfig &config)
    : max_entries_{config.max_entries}, config_{config} {
  // deque doesn't have reserve(), but we set max_entries for overflow control
}

//
// Public method implementations
//

template <typename Mutex>
std::vector<LogEntry> ImGUI_sink<Mutex>::get_entries() const {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  return std::vector<LogEntry>(buffer_.begin(), buffer_.end());
}

template <typename Mutex> size_t ImGUI_sink<Mutex>::get_entry_count() const {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  return buffer_.size();
}

template <typename Mutex> void ImGUI_sink<Mutex>::clear() {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  buffer_.clear();
}

template <typename Mutex> ImGUIConfig ImGUI_sink<Mutex>::get_config() const {
  return config_;
}

template <typename Mutex> bool ImGUI_sink<Mutex>::is_full() const {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  return buffer_.size() >= max_entries_;
}

//
// Protected method implementations
//

template <typename Mutex>
void ImGUI_sink<Mutex>::sink_it_(const spdlog::details::log_msg &msg) {
  // Create log entry from the message
  LogEntry entry;
  entry.timestamp = msg.time;
  entry.level = msg.level;

  // Format the message using the inherited formatter
  if (this->formatter_ != nullptr) {
    spdlog::memory_buf_t formatted;
    this->formatter_->format(msg, formatted);
    entry.message = std::string(formatted.data(), formatted.size());
  } else {
    // Fallback if no formatter is set
    entry.message = std::string(msg.payload.data(), msg.payload.size());
  }

  // Store thread ID as string representation for portability
  std::ostringstream thread_id_stream;
  thread_id_stream << msg.thread_id;
  entry.thread_id_str = thread_id_stream.str();

  // Extract source info if available
  if (msg.source.filename != nullptr) {
    std::ostringstream oss;
    oss << msg.source.filename << ":" << msg.source.line;
    entry.source_info = oss.str();
  }

  // Add to circular buffer with overflow handling
  // Note: base_sink's mutex is already held when sink_it_ is called
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  if (buffer_.size() >= max_entries_) {
    // Remove oldest entry to make room
    buffer_.pop_front();
  }
  buffer_.push_back(std::move(entry));
}

template <typename Mutex> void ImGUI_sink<Mutex>::flush_() {
  // No-op for in-memory buffer
  // The ImGUI sink stores data in memory and doesn't require explicit flushing
}

//
// Factory function implementations
//

std::shared_ptr<ImGUI_sink_mt> create_imgui_sink() {
  return std::make_shared<ImGUI_sink_mt>();
}

std::shared_ptr<ImGUI_sink_mt> create_imgui_sink(const ImGUIConfig &config) {
  return std::make_shared<ImGUI_sink_mt>(config);
}

// Explicit template instantiation for thread-safe and single-threaded versions
template class ImGUI_sink<std::mutex>;
template class ImGUI_sink<spdlog::details::null_mutex>;

} // namespace Logging
} // namespace TFCADIR
