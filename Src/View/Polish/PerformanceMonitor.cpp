#include "PerformanceMonitor.hpp"

namespace View {

PerformanceMonitor::PerformanceMonitor()
    : startTime_(std::chrono::steady_clock::now()), lastUpdate_(startTime_) {
  fpsHistory_.reserve(config_.rollingWindow);
  frameTimeHistory_.reserve(config_.rollingWindow);
}

PerformanceMonitor::~PerformanceMonitor() = default;

void PerformanceMonitor::updateFrame(float deltaTime) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Update frame count
  ++stats_.frameCount;

  // Calculate FPS and frame time
  if (deltaTime > 0.0f) {
    stats_.fps = 1.0f / deltaTime;
    stats_.frameTime = deltaTime * 1000.0f; // Convert to milliseconds
  } else {
    stats_.fps = 0.0f;
    stats_.frameTime = 0.0f;
  }

  // Update history for rolling averages
  fpsHistory_.push_back(stats_.fps);
  frameTimeHistory_.push_back(stats_.frameTime);

  // Maintain rolling window size
  if (fpsHistory_.size() > config_.rollingWindow) {
    fpsHistory_.erase(fpsHistory_.begin());
  }
  if (frameTimeHistory_.size() > config_.rollingWindow) {
    frameTimeHistory_.erase(frameTimeHistory_.begin());
  }

  // Update rolling average FPS
  stats_.fpsAvg = calculateRollingAverage(fpsHistory_);

  // Update min/max FPS
  updateMinMaxFPS();

  // Update uptime
  auto now = std::chrono::steady_clock::now();
  stats_.uptime =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
}

void PerformanceMonitor::updateMemory(float memoryMB) {
  std::lock_guard<std::mutex> lock(mutex_);
  stats_.memoryUsage = memoryMB;
}

PerformanceStats PerformanceMonitor::getStats() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stats_;
}

PerformanceDisplayConfig PerformanceMonitor::getDisplayConfig() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return config_;
}

void PerformanceMonitor::setDisplayConfig(
    const PerformanceDisplayConfig &config) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Resize history vectors if rolling window changed
  if (config.rollingWindow != config_.rollingWindow) {
    fpsHistory_.reserve(config.rollingWindow);
    frameTimeHistory_.reserve(config.rollingWindow);

    // Trim if new window is smaller
    while (fpsHistory_.size() > config.rollingWindow) {
      fpsHistory_.erase(fpsHistory_.begin());
    }
    while (frameTimeHistory_.size() > config.rollingWindow) {
      frameTimeHistory_.erase(frameTimeHistory_.begin());
    }
  }

  config_ = config;

  // Notify callback if set
  if (onConfigChanged_) {
    onConfigChanged_(config_);
  }
}

void PerformanceMonitor::setConfigChangedCallback(
    std::function<void(const PerformanceDisplayConfig &)> callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  onConfigChanged_ = std::move(callback);
}

void PerformanceMonitor::reset() {
  std::lock_guard<std::mutex> lock(mutex_);

  stats_ = PerformanceStats{};
  fpsHistory_.clear();
  frameTimeHistory_.clear();
  startTime_ = std::chrono::steady_clock::now();
  lastUpdate_ = startTime_;
}

void PerformanceMonitor::toggleOverlay() {
  std::lock_guard<std::mutex> lock(mutex_);
  config_.showOverlay = !config_.showOverlay;

  // Notify callback if set
  if (onConfigChanged_) {
    onConfigChanged_(config_);
  }
}

bool PerformanceMonitor::isOverlayVisible() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return config_.showOverlay;
}

float PerformanceMonitor::calculateRollingAverage(
    const std::vector<float> &history) const {
  if (history.empty()) {
    return 0.0f;
  }

  float sum = 0.0f;
  for (float value : history) {
    sum += value;
  }
  return sum / static_cast<float>(history.size());
}

void PerformanceMonitor::updateMinMaxFPS() {
  if (fpsHistory_.empty()) {
    stats_.fpsMin = 0.0f;
    stats_.fpsMax = 0.0f;
    return;
  }

  stats_.fpsMin = fpsHistory_.front();
  stats_.fpsMax = fpsHistory_.front();

  for (float fps : fpsHistory_) {
    if (fps < stats_.fpsMin) {
      stats_.fpsMin = fps;
    }
    if (fps > stats_.fpsMax) {
      stats_.fpsMax = fps;
    }
  }
}

bool PerformanceMonitor::shouldUpdateDisplay() {
  std::lock_guard<std::mutex> lock(mutex_);
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate_)
          .count();

  if (elapsed >= static_cast<long long>(config_.updateRate)) {
    lastUpdate_ = now;
    return true;
  }
  return false;
}

} // namespace View
