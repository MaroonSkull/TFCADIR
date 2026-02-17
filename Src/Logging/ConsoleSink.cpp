/**
 * @file ConsoleSink.cpp
 * @brief Implementation of the console sink for TFCADIR logging system
 */

#include "ConsoleSink.hpp"

#include <iostream>

#include <spdlog/spdlog.h>

namespace TFCADIR {
namespace Logging {

//
// Constructor implementations
//

template <typename Mutex> ConsoleSink<Mutex>::ConsoleSink() : m_config{} {
  // Set the level for this sink - only critical and above
  this->set_level(m_config.level);
}

template <typename Mutex>
ConsoleSink<Mutex>::ConsoleSink(const ConsoleSinkConfig &config)
    : m_config{config} {
  // Set the level for this sink
  this->set_level(m_config.level);
}

template <typename Mutex> ConsoleSink<Mutex>::~ConsoleSink() {
  try {
    flush_();
  } catch (...) {
    // Ignore exceptions in destructor
  }
}

//
// Public method implementations
//

template <typename Mutex>
ConsoleSinkConfig ConsoleSink<Mutex>::get_config() const {
  return m_config;
}

template <typename Mutex>
void ConsoleSink<Mutex>::set_config(const ConsoleSinkConfig &config) {
  m_config = config;
  this->set_level(m_config.level);
}

template <typename Mutex>
void ConsoleSink<Mutex>::set_level(spdlog::level::level_enum level) {
  m_config.level = level;
  spdlog::sinks::base_sink<Mutex>::set_level(level);
}

//
// Protected method implementations
//

template <typename Mutex>
void ConsoleSink<Mutex>::sink_it_(const spdlog::details::log_msg &msg) {
  // Format the message using the inherited formatter
  spdlog::memory_buf_t formatted;
  this->formatter_->format(msg, formatted);

  // Write to stderr for critical errors
  // Using stderr is appropriate for error messages as it's unbuffered
  // and separate from stdout
  std::cerr.write(formatted.data(),
                  static_cast<std::streamsize>(formatted.size()));
}

template <typename Mutex> void ConsoleSink<Mutex>::flush_() {
  // Flush stderr to ensure all output is written
  std::cerr.flush();
}

//
// Factory function implementations
//

std::shared_ptr<ConsoleSink_mt> create_console_sink() {
  return std::make_shared<ConsoleSink_mt>();
}

std::shared_ptr<ConsoleSink_mt>
create_console_sink(const ConsoleSinkConfig &config) {
  return std::make_shared<ConsoleSink_mt>(config);
}

// Explicit template instantiation for thread-safe and single-threaded versions
template class ConsoleSink<std::mutex>;
template class ConsoleSink<spdlog::details::null_mutex>;

} // namespace Logging
} // namespace TFCADIR
