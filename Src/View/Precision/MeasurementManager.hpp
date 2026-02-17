#pragma once

#include <Model/IModel.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace view {

// ==========================================================================
// Phase 6: Precision & Snapping - MeasurementManager
// ==========================================================================

/**
 * @brief Figure type enumeration for measurement calculations
 */
enum class FigureType {
  Triangle, ///< Triangle figure
  Quad,     ///< Quadrilateral figure
  Circle,   ///< Circle figure
  Ngon,     ///< N-sided polygon figure
  Line,     ///< Line figure
  Unknown   ///< Unknown figure type
};

/**
 * @brief Result structure for real-time measurement calculations
 */
struct MeasurementResult {
  /// Distance measurement (if applicable)
  std::optional<float> distance;

  /// Angle measurement in degrees (if applicable)
  std::optional<float> angle;

  /// Area measurement (if applicable)
  std::optional<float> area;

  /// Perimeter measurement (if applicable)
  std::optional<float> perimeter;

  /// Whether the result contains valid data
  bool isValid = false;
};

/**
 * @brief Coordinates measurement calculations for precision drawing
 *
 * DESIGN RATIONALE:
 *
 * 1. Stateless Coordinator Pattern:
 *    - MeasurementManager maintains no measurement state itself
 *    - All measurement settings are stored in UIFSMAdapter
 *    - This follows the same pattern as GridManager and SnapManager
 *
 * 2. Caching with Dirty Flag:
 *    - measurementsDirty_ flag tracks when cached calculations need refresh
 *    - Set to true when measurement settings change
 *    - Cleared after UI panels query the current state
 *    - This prevents expensive recalculations on every frame
 *
 * 3. Callback Registration:
 *    - Constructor registers callback with UIFSMAdapter
 *    - Callback sets measurementsDirty_ = true when settings change
 *    - This ensures UI panels always know when to refresh measurements
 *
 * 4. State Query Delegation:
 *    - All state query methods delegate to UIFSMAdapter
 *    - UIFSMAdapter is the single source of truth for measurement settings
 *    - This maintains consistency across the application
 *
 * USAGE:
 *
 * UI panels should:
 * 1. Check isMeasurementsDirty() to see if refresh is needed
 * 2. Call getMeasurementSettings() and other query methods
 * 3. Call clearMeasurementsDirty() after updating display
 * 4. Use calculateDistance(), calculateAngle(), etc. for calculations
 */
class MeasurementManager {
private:
  /// Reference to the UI FSM adapter (non-owning)
  UIFSMAdapter &uiFSMAdapter_;

  /// Cache invalidation flag for measurement calculations
  mutable bool measurementsDirty_;

public:
  /**
   * @brief Constructs a MeasurementManager
   * @param uiFSMAdapter Reference to the UI FSM adapter
   *
   * Registers a callback with UIFSMAdapter to set measurementsDirty_ when
   * measurement settings change.
   */
  explicit MeasurementManager(UIFSMAdapter &uiFSMAdapter);

  // ==========================================================================
  // State Query Methods (delegate to UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Get the current measurement settings
   * @return Current measurement settings from UIFSMAdapter
   */
  [[nodiscard]] MeasurementSettings getMeasurementSettings() const;

  /**
   * @brief Check if show measurements is enabled
   * @return true if measurements should be displayed
   */
  [[nodiscard]] bool isShowMeasurementsEnabled() const;

  /**
   * @brief Check if real-time measurement is enabled
   * @return true if real-time measurement feedback should be shown
   */
  [[nodiscard]] bool isRealTimeMeasurementEnabled() const;

  // ==========================================================================
  // Dirty Flag Management
  // ==========================================================================

  /**
   * @brief Check if measurements are dirty (need recalculation)
   * @return true if cached measurements need to be refreshed
   */
  [[nodiscard]] bool isMeasurementsDirty() const;

  /**
   * @brief Clear the measurements dirty flag
   *
   * Should be called by UI panels after they have refreshed their display.
   */
  void clearMeasurementsDirty();

  // ==========================================================================
  // Distance Calculation
  // ==========================================================================

  /**
   * @brief Calculate Euclidean distance between two points
   * @param from Starting point
   * @param to Ending point
   * @return Distance in world units
   */
  [[nodiscard]] float calculateDistance(const glm::vec2 &from,
                                        const glm::vec2 &to) const;

  // ==========================================================================
  // Angle Calculation
  // ==========================================================================

  /**
   * @brief Calculate angle between two lines
   * @param line1Start Starting point of first line
   * @param line1End Ending point of first line
   * @param line2Start Starting point of second line
   * @param line2End Ending point of second line
   * @return Angle in degrees (0-180°)
   */
  [[nodiscard]] float calculateAngle(const glm::vec2 &line1Start,
                                     const glm::vec2 &line1End,
                                     const glm::vec2 &line2Start,
                                     const glm::vec2 &line2End) const;

  // ==========================================================================
  // Area Calculation
  // ==========================================================================

  /**
   * @brief Calculate area of a figure
   * @param figure Shared pointer to the figure
   * @return Area in square world units
   *
   * Supports Triangle, Quad, Circle, and Ngon figure types.
   * Uses appropriate formula for each figure type.
   */
  [[nodiscard]] float
  calculateArea(const std::shared_ptr<model::IFigure> &figure) const;

  // ==========================================================================
  // Perimeter Calculation
  // ==========================================================================

  /**
   * @brief Calculate perimeter of a figure
   * @param figure Shared pointer to the figure
   * @return Perimeter in world units
   *
   * Supports all figure types.
   * For Circle, returns circumference (2 * PI * radius).
   */
  [[nodiscard]] float
  calculatePerimeter(const std::shared_ptr<model::IFigure> &figure) const;

  // ==========================================================================
  // Real-time Measurement During Drawing
  // ==========================================================================

  /**
   * @brief Calculate measurements for partial figure during drawing
   * @param points Vector of points collected so far
   * @param type Type of figure being drawn
   * @return MeasurementResult with all applicable measurements
   *
   * Provides real-time feedback during drawing operations.
   * Returns distance, angle, area, and perimeter based on available points.
   */
  [[nodiscard]] MeasurementResult
  calculateRealTimeMeasurement(const std::vector<glm::vec2> &points,
                               FigureType type) const;

  // ==========================================================================
  // Last Measurement Storage (for MeasurementDisplay)
  // ==========================================================================

  /**
   * @brief Set the last measurement result
   * @param result The measurement result to store
   *
   * Stores the most recent measurement result for display purposes.
   */
  void setLastMeasurement(const MeasurementResult &result);

  /**
   * @brief Get the last measurement result
   * @return The most recent measurement result
   */
  [[nodiscard]] const MeasurementResult &getLastMeasurement() const;

private:
  /// Last measurement result for display
  mutable MeasurementResult lastMeasurement_;
};

} // namespace view
