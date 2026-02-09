#pragma once

#include <View/Precision/GridManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Measurement type enumeration
 *
 * Defines the types of measurements that can be calculated
 * by the MeasurementManager.
 */
enum class Type { Distance, Angle, Area };

/**
 * @brief Measurement result structure
 *
 * Contains all information about a calculated measurement,
 * including its type, value, units, and formatted display string.
 * Used by UI components to display measurement information to users.
 */
struct MeasurementResult {
  /// Type of measurement (distance, angle, area)
  Type type;

  /// Measured value in the appropriate units
  float value;

  /// Unit string (e.g., "mm", "deg", "mm²")
  std::string unit;

  /// Points used for the measurement
  std::vector<glm::vec3> points;

  /// Whether the measurement is valid
  bool isValid;

  /// Formatted string for display (e.g., "50.00 mm")
  std::string formattedString;

  /// Default constructor
  MeasurementResult() : type(Type::Distance), value(0.0f), isValid(false) {}

  /// Full constructor with all fields
  MeasurementResult(Type t, float v, const std::string &u,
                    const std::vector<glm::vec3> &p, bool valid,
                    const std::string &f)
      : type(t), value(v), unit(u), points(p), isValid(valid),
        formattedString(f) {}
};

/**
 * @brief Manager for measurement calculations
 *
 * Provides measurement functionality for distance, angle, and area
 * calculations. Follows the stateless coordinator pattern - no local
 * state storage beyond performance cache.
 *
 * MeasurementManager is responsible for:
 * - Calculating distance between points
 * - Calculating angles between three points
 * - Calculating area of polygons
 * - Providing real-time measurement feedback
 * - Formatting measurements for display
 */
class MeasurementManager {
public:
  /**
   * @brief Construct a new Measurement Manager object
   * @param fsmAdapter Reference to UIFSMAdapter for state queries
   * @param gridManager Reference to GridManager for grid calculations
   *
   * All references are stored for state queries. MeasurementManager
   * does not manage the lifecycle of these components.
   */
  MeasurementManager(UIFSMAdapter &fsmAdapter, GridManager &gridManager);

  /**
   * @brief Destroy the Measurement Manager object
   */
  ~MeasurementManager() = default;

  // Distance measurements

  /**
   * @brief Measure distance between two points
   * @param p1 First point
   * @param p2 Second point
   * @return Distance in millimeters
   *
   * Calculates Euclidean distance between two 3D points.
   */
  float measureDistance(const glm::vec3 &p1, const glm::vec3 &p2) const;

  // Angle measurements

  /**
   * @brief Measure angle at vertex between two points
   * @param p1 First point defining one arm of angle
   * @param p2 Vertex point (center of angle)
   * @param p3 Second point defining other arm of angle
   * @return Angle in degrees (0-180)
   *
   * Calculates the angle between two vectors formed by
   * (p1 - p2) and (p3 - p2).
   *
   * To measure the angle at p2 formed by three points (p1, p2, p3),
   * call: measureAngle(p1, p2, p3)
   */
  float measureAngle(const glm::vec3 &p1, const glm::vec3 &p2,
                     const glm::vec3 &p3) const;

  // Area measurements

  /**
   * @brief Measure area of polygon defined by points
   * @param points Vector of points defining polygon vertices
   * @return Area in square millimeters
   *
   * Uses the Shoelace formula to calculate polygon area.
   * Returns 0.0f if points vector has fewer than 3 elements.
   */
  float measureArea(const std::vector<glm::vec3> &points) const;

  // Real-time measurement feedback

  /**
   * @brief Get real-time measurement during drawing operations
   * @param collectedPoints Points already collected by the tool
   * @param currentCursorPos Current cursor position in world coordinates
   * @param toolId Identifier of the current tool (e.g., "Line3D", "Circle3D")
   * @return MeasurementResult with calculated measurement
   *
   * Calculates appropriate measurement based on tool type and collected points.
   * Uses cache for performance optimization.
   */
  MeasurementResult
  getRealtimeMeasurement(const std::vector<glm::vec3> &collectedPoints,
                         const glm::vec3 &currentCursorPos,
                         const std::string &toolId) const;

  /**
   * @brief Format a measurement value with units
   * @param value Measurement value
   * @param unit Unit string
   * @return Formatted string (e.g., "50.00 mm")
   *
   * Formats the measurement value with default precision
   * and appends the unit string.
   */
  std::string formatMeasurement(float value, const std::string &unit) const;

private:
  /// Reference to UIFSMAdapter for state queries
  UIFSMAdapter &fsmAdapter_;

  /// Reference to GridManager for grid calculations
  GridManager &gridManager_;

  // NO local state (all calculations are stateless)
  // Performance cache for real-time measurements
  mutable struct MeasurementCache {
    MeasurementResult result;
    std::vector<glm::vec3> lastPoints;
    bool valid = false;
  } measurementCache_;

  /**
   * @brief Invalidate the measurement cache
   *
   * Called when collected points change or tool changes.
   * Forces recalculation on next getRealtimeMeasurement call.
   */
  void invalidateCache();

  // Helper methods

  /**
   * @brief Calculate distance between two points
   * @param a First point
   * @param b Second point
   * @return Euclidean distance
   */
  static float distance(const glm::vec3 &a, const glm::vec3 &b);

  /**
   * @brief Convert radians to degrees
   * @param radians Angle in radians
   * @return Angle in degrees
   */
  static float toDegrees(float radians);

  /**
   * @brief Calculate polygon area using Shoelace formula
   * @param points Vector of points defining polygon vertices
   * @return Area in square millimeters
   *
   * Projects points onto XY plane and applies Shoelace formula.
   * Returns 0.0f if points vector has fewer than 3 elements.
   */
  float calculatePolygonArea(const std::vector<glm::vec3> &points) const;

  /**
   * @brief Get unit string for measurement type
   * @param type Measurement type
   * @return Unit string (e.g., "mm", "deg", "mm²")
   */
  static std::string getUnitForType(Type type);
};

} // namespace view
