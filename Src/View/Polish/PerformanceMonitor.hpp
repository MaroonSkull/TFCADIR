#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace View {

/**
 * @brief Performance monitoring statistics
 */
struct PerformanceStats {
  float fps = 0.0f;                    ///< Current FPS
  float frameTime = 0.0f;              ///< Frame time in milliseconds
  float fpsMin = 0.0f;                 ///< Minimum FPS (rolling window)
  float fpsMax = 0.0f;                 ///< Maximum FPS (rolling window)
  float fpsAvg = 0.0f;                 ///< Average FPS (rolling window)
  float memoryUsage = 0.0f;            ///< Memory usage in MB
  float cpuUsage = 0.0f;               ///< CPU usage percentage (optional)
  size_t frameCount = 0;               ///< Total frames rendered
  std::chrono::milliseconds uptime{0}; ///< Application uptime
};

/**
 * @brief Performance display configuration
 */
struct PerformanceDisplayConfig {
  bool showOverlay = true;    ///< Show performance overlay
  bool showCompact = false;   ///< Use compact display mode
  bool showMemory = true;     ///< Show memory usage
  bool showCPU = false;       ///< Show CPU usage (platform-dependent)
  bool showHistogram = false; ///< Show frame time histogram
  int position = 0;           ///< 0=TL, 1=TR, 2=BL, 3=BR
  float updateRate = 100.0f;  ///< Update rate in milliseconds
  size_t rollingWindow = 60;  ///< Rolling average window size
};

/**
 * @brief Performance monitor for real-time statistics
 *
 * Tracks FPS, frame time, memory usage, and provides configurable overlay
 * display. Thread-safe statistics collection with mutex protection.
 */
class PerformanceMonitor {
public:
  /**
   * @brief Construct performance monitor
   */
  PerformanceMonitor();

  /**
   * @brief Destroy performance monitor
   */
  ~PerformanceMonitor();

  // Delete copy constructor and copy assignment
  PerformanceMonitor(const PerformanceMonitor &) = delete;
  PerformanceMonitor &operator=(const PerformanceMonitor &) = delete;

  /**
   * @brief Update frame statistics (call once per frame)
   * @param deltaTime Time since last frame in seconds
   */
  void updateFrame(float deltaTime);

  /**
   * @brief Update memory statistics
   * @param memoryMB Memory usage in megabytes
   */
  void updateMemory(float memoryMB);

  /**
   * @brief Get current performance statistics
   * @return PerformanceStats Current statistics
   */
  PerformanceStats getStats() const;

  /**
   * @brief Get display configuration
   * @return PerformanceDisplayConfig Current configuration
   */
  PerformanceDisplayConfig getDisplayConfig() const;

  /**
   * @brief Set display configuration
   * @param config New configuration
   */
  void setDisplayConfig(const PerformanceDisplayConfig &config);

  /**
   * @brief Set configuration changed callback
   * @param callback Callback function
   */
  void setConfigChangedCallback(
      std::function<void(const PerformanceDisplayConfig &)> callback);

  /**
   * @brief Reset statistics
   */
  void reset();

  /**
   * @brief Toggle overlay visibility
   */
  void toggleOverlay();

  /**
   * @brief Check if overlay should be visible
   * @return true if overlay is visible
   */
  bool isOverlayVisible() const;

private:
  mutable std::mutex mutex_;
  PerformanceStats stats_;
  PerformanceDisplayConfig config_;
  std::function<void(const PerformanceDisplayConfig &)> onConfigChanged_;

  std::vector<float> fpsHistory_;
  std::vector<float> frameTimeHistory_;
  std::chrono::steady_clock::time_point startTime_;
  std::chrono::steady_clock::time_point lastUpdate_;

  /**
   * @brief Calculate rolling average
   * @param history History vector to calculate average from
   * @return Rolling average value
   */
  float calculateRollingAverage(const std::vector<float> &history) const;

  /**
   * @brief Update min/max FPS from history
   */
  void updateMinMaxFPS();

  /**
   * @brief Check if display should update based on rate limiting
   * @return true if display should update
   */
  bool shouldUpdateDisplay();
};

} // namespace View
