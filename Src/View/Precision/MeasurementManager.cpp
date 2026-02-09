#include <View/Precision/MeasurementManager.hpp>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace view {

// Constants for measurement calculations
namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float DEGREES_PER_RADIAN = 180.0f / PI;
} // namespace

// ============================================================================
// Constructor / Destructor
// ============================================================================

MeasurementManager::MeasurementManager(UIFSMAdapter &fsmAdapter,
                                       GridManager &gridManager)
    : fsmAdapter_(fsmAdapter), gridManager_(gridManager) {}

// ============================================================================
// Distance Measurements
// ============================================================================

float MeasurementManager::measureDistance(const glm::vec3 &p1,
                                          const glm::vec3 &p2) const {
  return distance(p1, p2);
}

// ============================================================================
// Angle Measurements
// ============================================================================

float MeasurementManager::measureAngle(const glm::vec3 &p1, const glm::vec3 &p2,
                                       const glm::vec3 &p3) const {
  // Calculate vectors from p2 (vertex) to p1 and p3
  glm::vec3 v1 = p1 - p2;
  glm::vec3 v2 = p3 - p2;

  // Calculate magnitudes
  float len1 = glm::length(v1);
  float len2 = glm::length(v2);

  // Check for zero-length vectors
  if (len1 < std::numeric_limits<float>::epsilon() ||
      len2 < std::numeric_limits<float>::epsilon()) {
    return 0.0f;
  }

  // Normalize vectors
  v1 /= len1;
  v2 /= len2;

  // Calculate dot product
  float dotProduct = glm::dot(v1, v2);

  // Clamp to [-1, 1] to avoid numerical errors
  dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);

  // Calculate angle in radians
  float angleRadians = std::acos(dotProduct);

  // Convert to degrees
  return toDegrees(angleRadians);
}

// ============================================================================
// Area Measurements
// ============================================================================

float MeasurementManager::measureArea(
    const std::vector<glm::vec3> &points) const {
  return calculatePolygonArea(points);
}

// ============================================================================
// Real-time Measurement Feedback
// ============================================================================

MeasurementResult MeasurementManager::getRealtimeMeasurement(
    const std::vector<glm::vec3> &collectedPoints,
    const glm::vec3 &currentCursorPos, const std::string &toolId) const {

  // Check if cache is valid
  if (measurementCache_.valid &&
      measurementCache_.lastPoints == collectedPoints) {
    return measurementCache_.result;
  }

  MeasurementResult result;
  result.isValid = false;

  // Determine measurement type based on tool and collected points
  if (toolId == "Line3D" || toolId == "Rectangle3D") {
    if (collectedPoints.size() >= 1) {
      result.type = Type::Distance;
      result.value = measureDistance(collectedPoints[0], currentCursorPos);
      result.unit = getUnitForType(Type::Distance);
      result.points = {collectedPoints[0], currentCursorPos};
      result.isValid = true;
    }
  } else if (toolId == "Circle3D") {
    if (collectedPoints.size() >= 1) {
      result.type = Type::Distance;
      result.value = measureDistance(collectedPoints[0], currentCursorPos);
      result.unit = getUnitForType(Type::Distance);
      result.points = {collectedPoints[0], currentCursorPos};
      result.isValid = true;
    }
  } else if (toolId == "Arc3D") {
    if (collectedPoints.size() >= 2) {
      result.type = Type::Angle;
      result.value = measureAngle(collectedPoints[0], currentCursorPos,
                                  collectedPoints[1]);
      result.unit = getUnitForType(Type::Angle);
      result.points = {collectedPoints[0], currentCursorPos,
                       collectedPoints[1]};
      result.isValid = true;
    }
  } else if (toolId == "Polygon3D") {
    if (collectedPoints.size() >= 2) {
      std::vector<glm::vec3> allPoints = collectedPoints;
      allPoints.push_back(currentCursorPos);
      result.type = Type::Area;
      result.value = measureArea(allPoints);
      result.unit = getUnitForType(Type::Area);
      result.points = allPoints;
      result.isValid = true;
    }
  }

  // Format the measurement
  if (result.isValid) {
    result.formattedString = formatMeasurement(result.value, result.unit);
  }

  // Update cache (mutable allows modification in const method for performance)
  MeasurementCache &cache = measurementCache_;
  cache.result = result;
  cache.lastPoints = collectedPoints;
  cache.valid = true;

  return result;
}

std::string
MeasurementManager::formatMeasurement(float value,
                                      const std::string &unit) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << value << " " << unit;
  return oss.str();
}

// ============================================================================
// Private Helper Methods
// ============================================================================

float MeasurementManager::distance(const glm::vec3 &a, const glm::vec3 &b) {
  return glm::length(b - a);
}

float MeasurementManager::toDegrees(float radians) {
  return radians * DEGREES_PER_RADIAN;
}

float MeasurementManager::calculatePolygonArea(
    const std::vector<glm::vec3> &points) const {
  if (points.size() < 3) {
    return 0.0f;
  }

  // Use Shoelace formula on XY plane
  float area = 0.0f;
  const size_t n = points.size();

  for (size_t i = 0; i < n; ++i) {
    size_t j = (i + 1) % n;
    area += points[i].x * points[j].y;
    area -= points[j].x * points[i].y;
  }

  return std::abs(area) * 0.5f;
}

std::string MeasurementManager::getUnitForType(Type type) {
  switch (type) {
  case Type::Distance:
    return "mm";
  case Type::Angle:
    return "deg";
  case Type::Area:
    return "mm²";
  default:
    return "";
  }
}

void MeasurementManager::invalidateCache() {
  measurementCache_.valid = false;
  measurementCache_.lastPoints.clear();
}

} // namespace view
