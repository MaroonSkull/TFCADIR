/**
 * @file FileSink.cpp
 * @brief Implementation of the rotating file sink for TFCADIR logging system
 */

#include "FileSink.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace TFCADIR {
namespace Logging {

// Default configuration constants
namespace {
/// Default log directory name
constexpr const char *DEFAULT_LOG_DIR = "logs";

/// Default log file name
constexpr const char *DEFAULT_LOG_FILENAME = "tfcadir.log";
} // namespace

//
// Constructor implementations
//

template <typename Mutex> FileSink<Mutex>::FileSink() : m_config{} {
  // Set default filename with logs/ directory
  m_config.base_filename =
      std::string(DEFAULT_LOG_DIR) + "/" + DEFAULT_LOG_FILENAME;
  initialize_();
}

template <typename Mutex>
FileSink<Mutex>::FileSink(const FileSinkConfig &config) : m_config{config} {
  initialize_();
}

template <typename Mutex> FileSink<Mutex>::~FileSink() {
  try {
    flush_();
  } catch (...) {
    // Ignore exceptions in destructor
  }
}

//
// Public method implementations
//

template <typename Mutex> void FileSink<Mutex>::rotate() {
  std::lock_guard<Mutex> lock(this->mutex_);
  rotate_();
}

template <typename Mutex>
size_t FileSink<Mutex>::get_current_file_size() const {
  return m_current_size;
}

template <typename Mutex> size_t FileSink<Mutex>::get_total_size() const {
  size_t total_size = m_current_size;

  try {
    // Get directory and base filename
    std::filesystem::path filepath(m_config.base_filename);
    std::filesystem::path directory = filepath.parent_path();
    std::string stem = filepath.stem().string();
    std::string ext = filepath.extension().string();

    if (directory.empty()) {
      directory = ".";
    }

    // Sum up sizes of all rotated files
    for (size_t i = 1; i <= m_config.max_files; ++i) {
      std::string rotated_name = stem + "." + std::to_string(i) + ext;
      std::filesystem::path rotated_path = directory / rotated_name;

      if (std::filesystem::exists(rotated_path)) {
        total_size += std::filesystem::file_size(rotated_path);
      }
    }
  } catch (const std::filesystem::filesystem_error &) {
    // Return what we have if filesystem operations fail
  }

  return total_size;
}

template <typename Mutex> FileSinkConfig FileSink<Mutex>::get_config() const {
  return m_config;
}

template <typename Mutex>
void FileSink<Mutex>::set_flush_level(spdlog::level::level_enum level) {
  m_config.flush_level = level;
}

template <typename Mutex>
spdlog::level::level_enum FileSink<Mutex>::get_flush_level() const {
  return m_config.flush_level;
}

//
// Protected method implementations
//

template <typename Mutex>
void FileSink<Mutex>::sink_it_(const spdlog::details::log_msg &msg) {
  if (!m_rotating_sink) {
    return;
  }

  // Format the message using the inherited formatter
  spdlog::memory_buf_t formatted;
  this->formatter_->format(msg, formatted);

  // Get the formatted string size
  size_t msg_size = formatted.size();

  // Write to the underlying rotating sink
  // The rotating_file_sink handles size checking and rotation internally
  m_rotating_sink->log(msg);

  // Update current size tracking
  m_current_size += msg_size;

  // Check if we need immediate flush for error/critical messages
  if (msg.level >= m_config.flush_level) {
    m_rotating_sink->flush();
  }
}

template <typename Mutex> void FileSink<Mutex>::flush_() {
  if (m_rotating_sink) {
    m_rotating_sink->flush();
  }
}

//
// Private method implementations
//

template <typename Mutex> void FileSink<Mutex>::initialize_() {
  // Ensure the log directory exists
  if (!ensure_log_directory_exists_()) {
    throw spdlog::spdlog_ex("Failed to create log directory: " +
                            m_config.base_filename);
  }

  // Create the underlying rotating file sink
  try {
    m_rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        m_config.base_filename, m_config.max_file_size, m_config.max_files);

    // Note: flush_on is a logger-level setting, not sink-level
    // The flush level is handled in sink_it_() for this custom sink

    // Initialize current size from existing file if any
    std::filesystem::path filepath(m_config.base_filename);
    if (std::filesystem::exists(filepath)) {
      m_current_size = std::filesystem::file_size(filepath);
    } else {
      m_current_size = 0;
    }
  } catch (const spdlog::spdlog_ex &ex) {
    throw;
  } catch (const std::exception &ex) {
    throw spdlog::spdlog_ex(std::string("Failed to initialize file sink: ") +
                            ex.what());
  }
}

template <typename Mutex> void FileSink<Mutex>::rotate_() {
  if (!m_rotating_sink) {
    return;
  }

  // Flush before rotation
  m_rotating_sink->flush();

  // The rotating_file_sink handles rotation automatically when size limit is
  // reached For manual rotation, we need to force it by creating a new sink
  // This is a workaround since spdlog's rotating_file_sink doesn't expose
  // manual rotation

  try {
    // Get the file path
    std::filesystem::path filepath(m_config.base_filename);
    std::filesystem::path directory = filepath.parent_path();
    std::string stem = filepath.stem().string();
    std::string ext = filepath.extension().string();

    if (directory.empty()) {
      directory = ".";
    }

    // Delete the oldest file if it exists
    std::string oldest_name =
        stem + "." + std::to_string(m_config.max_files) + ext;
    std::filesystem::path oldest_path = directory / oldest_name;

    if (std::filesystem::exists(oldest_path)) {
      std::filesystem::remove(oldest_path);
    }

    // Rotate existing files
    for (size_t i = m_config.max_files; i > 1; --i) {
      std::string current_name = stem + "." + std::to_string(i - 1) + ext;
      std::string next_name = stem + "." + std::to_string(i) + ext;

      std::filesystem::path current_path = directory / current_name;
      std::filesystem::path next_path = directory / next_name;

      if (std::filesystem::exists(current_path)) {
        std::filesystem::rename(current_path, next_path);
      }
    }

    // Rename current file to .1
    if (std::filesystem::exists(filepath)) {
      std::string first_rotated = stem + ".1" + ext;
      std::filesystem::path first_path = directory / first_rotated;
      std::filesystem::rename(filepath, first_path);
    }

    // Create a new empty file
    std::ofstream new_file(filepath, std::ios::out | std::ios::trunc);
    new_file.close();

    // Reset current size
    m_current_size = 0;

    // Recreate the rotating sink to reset its internal state
    m_rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        m_config.base_filename, m_config.max_file_size, m_config.max_files);
    // Note: flush_on is a logger-level setting, not sink-level
  } catch (const std::filesystem::filesystem_error &ex) {
    spdlog::error("Failed to rotate log file: {}", ex.what());
  }
}

template <typename Mutex> bool FileSink<Mutex>::ensure_log_directory_exists_() {
  try {
    std::filesystem::path filepath(m_config.base_filename);
    std::filesystem::path directory = filepath.parent_path();

    if (directory.empty()) {
      return true; // Current directory, no need to create
    }

    if (!std::filesystem::exists(directory)) {
      std::error_code ec;
      return std::filesystem::create_directories(directory, ec);
    }

    return true;
  } catch (const std::filesystem::filesystem_error &) {
    return false;
  }
}

//
// Factory function implementations
//

std::shared_ptr<FileSink_mt> create_file_sink() {
  auto sink = std::make_shared<FileSink_mt>();
  // Set up periodic flush with default configuration flush interval
  if (sink->get_config().flush_interval.count() > 0) {
    spdlog::flush_every(
        std::chrono::seconds(sink->get_config().flush_interval.count()));
  }
  return sink;
}

std::shared_ptr<FileSink_mt> create_file_sink(const FileSinkConfig &config) {
  auto sink = std::make_shared<FileSink_mt>(config);
  // Set up periodic flush based on configuration
  if (config.flush_interval.count() > 0) {
    spdlog::flush_every(std::chrono::seconds(config.flush_interval.count()));
  }
  return sink;
}

// Explicit template instantiation for thread-safe and single-threaded versions
template class FileSink<std::mutex>;
template class FileSink<spdlog::details::null_mutex>;

} // namespace Logging
} // namespace TFCADIR
