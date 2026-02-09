#include <Model/FlatFigure.hpp>
#include <View/Precision/MeasurementManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace view {

// ==========================================================================
// Architecture Documentation
// ==========================================================================

/**
 * @class MeasurementManager
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

MeasurementManager::MeasurementManager(UIFSMAdapter &uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter), measurementsDirty_(true) {
  // Register callback for measurement settings changes
  uiFSMAdapter_.setMeasurementSettingsChangedCallback(
      [this]() { measurementsDirty_ = true; });
}

// ==========================================================================
// State Query Methods (delegate to UIFSMAdapter)
// ==========================================================================

MeasurementSettings MeasurementManager::getMeasurementSettings() const {
  return uiFSMAdapter_.getMeasurementSettings();
}

bool MeasurementManager::isShowMeasurementsEnabled() const {
  return uiFSMAdapter_.isShowMeasurementsEnabled();
}

bool MeasurementManager::isRealTimeMeasurementEnabled() const {
  return uiFSMAdapter_.isRealTimeMeasurementEnabled();
}

// ==========================================================================
// Dirty Flag Management
// ==========================================================================

bool MeasurementManager::isMeasurementsDirty() const {
  return measurementsDirty_;
}

void MeasurementManager::clearMeasurementsDirty() {
  measurementsDirty_ = false;
}

// ==========================================================================
// Distance Calculation
// ==========================================================================

float MeasurementManager::calculateDistance(const glm::vec2 &from,
                                            const glm::vec2 &to) const {
  return glm::distance(from, to);
}

// ==========================================================================
// Angle Calculation
// ==========================================================================

float MeasurementManager::calculateAngle(const glm::vec2 &line1Start,
                                         const glm::vec2 &line1End,
                                         const glm::vec2 &line2Start,
                                         const glm::vec2 &line2End) const {
  // Calculate direction vectors
  glm::vec2 dir1 = glm::normalize(line1End - line1Start);
  glm::vec2 dir2 = glm::normalize(line2End - line2Start);

  // Calculate dot product
  float dot = glm::dot(dir1, dir2);

  // Clamp to [-1, 1] to avoid numerical errors
  dot = glm::clamp(dot, -1.0f, 1.0f);

  // Calculate angle in radians and convert to degrees
  float angleRadians = glm::acos(dot);
  return angleRadians * 180.0f / glm::pi<float>();
}

// ==========================================================================
// Area Calculation
// ==========================================================================

float MeasurementManager::calculateArea(
    const std::shared_ptr<model::IFigure> &figure) const {
  if (!figure) {
    return 0.0f;
  }

  // Check Triangle
  if (auto tri =
          std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    // Area of triangle using cross product formula
    // Area = 0.5 * |(B - A) x (C - A)|
    glm::vec2 v1(tri->second.x - tri->first.x, tri->second.y - tri->first.y);
    glm::vec2 v2(tri->third.x - tri->first.x, tri->third.y - tri->first.y);
    float cross = v1.x * v2.y - v1.y * v2.x;
    return 0.5f * std::abs(cross);
  }

  // Check Quad
  if (auto quad =
          std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
    // Split quad into two triangles and sum their areas
    // Triangle 1: vertices 0, 1, 2
    // Triangle 2: vertices 0, 2, 3
    glm::vec2 v1a(quad->second.x - quad->first.x,
                  quad->second.y - quad->first.y);
    glm::vec2 v2a(quad->third.x - quad->first.x, quad->third.y - quad->first.y);
    float cross1 = v1a.x * v2a.y - v1a.y * v2a.x;

    glm::vec2 v1b(quad->third.x - quad->first.x, quad->third.y - quad->first.y);
    glm::vec2 v2b(quad->fourth.x - quad->first.x,
                  quad->fourth.y - quad->first.y);
    float cross2 = v1b.x * v2b.y - v1b.y * v2b.x;

    return 0.5f * (std::abs(cross1) + std::abs(cross2));
  }

  // Check Circle
  if (auto circle =
          std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    // Area = PI * r^2
    return glm::pi<float>() * circle->radius * circle->radius;
  }

  // Check Ngon
  if (auto ngon =
          std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
    // Area of regular polygon: (n * s^2) / (4 * tan(PI/n))
    // where n = number of sides, s = side length
    float n = ngon->n;
    if (n < 3.0f) {
      return 0.0f;
    }

    // Calculate side length from radius
    // For a regular polygon: s = 2 * r * sin(PI/n)
    float sideLength = 2.0f * ngon->radius * glm::sin(glm::pi<float>() / n);

    // Calculate area using regular polygon formula
    float angle = glm::pi<float>() / n;
    float area = (n * sideLength * sideLength) / (4.0f * glm::tan(angle));

    return area;
  }

  return 0.0f;
}

// ==========================================================================
// Perimeter Calculation
// ==========================================================================

float MeasurementManager::calculatePerimeter(
    const std::shared_ptr<model::IFigure> &figure) const {
  if (!figure) {
    return 0.0f;
  }

  // Check Circle
  if (auto circle =
          std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    // Perimeter (circumference) = 2 * PI * r
    return 2.0f * glm::pi<float>() * circle->radius;
  }

  // Check Ngon
  if (auto ngon =
          std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
    // Perimeter = n * s (number of sides * side length)
    float n = ngon->n;
    // Calculate side length from radius
    float sideLength = 2.0f * ngon->radius * glm::sin(glm::pi<float>() / n);
    return n * sideLength;
  }

  // Check Triangle
  if (auto tri =
          std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    // Sum of all edge lengths
    float perimeter = 0.0f;
    perimeter += glm::distance(glm::vec2(tri->first.x, tri->first.y),
                               glm::vec2(tri->second.x, tri->second.y));
    perimeter += glm::distance(glm::vec2(tri->second.x, tri->second.y),
                               glm::vec2(tri->third.x, tri->third.y));
    perimeter += glm::distance(glm::vec2(tri->third.x, tri->third.y),
                               glm::vec2(tri->first.x, tri->first.y));
    return perimeter;
  }

  // Check Quad
  if (auto quad =
          std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
    // Sum of all edge lengths
    float perimeter = 0.0f;
    perimeter += glm::distance(glm::vec2(quad->first.x, quad->first.y),
                               glm::vec2(quad->second.x, quad->second.y));
    perimeter += glm::distance(glm::vec2(quad->second.x, quad->second.y),
                               glm::vec2(quad->third.x, quad->third.y));
    perimeter += glm::distance(glm::vec2(quad->third.x, quad->third.y),
                               glm::vec2(quad->fourth.x, quad->fourth.y));
    perimeter += glm::distance(glm::vec2(quad->fourth.x, quad->fourth.y),
                               glm::vec2(quad->first.x, quad->first.y));
    return perimeter;
  }

  return 0.0f;
}

// ==========================================================================
// Real-time Measurement During Drawing
// ==========================================================================

MeasurementResult MeasurementManager::calculateRealTimeMeasurement(
    const std::vector<glm::vec2> &points, FigureType type) const {
  MeasurementResult result;

  if (points.empty()) {
    result.isValid = false;
    return result;
  }

  result.isValid = true;

  // Calculate distance if we have at least 2 points
  if (points.size() >= 2) {
    result.distance = calculateDistance(points[0], points[1]);
  }

  // Calculate angle if we have at least 4 points (2 lines)
  if (points.size() >= 4) {
    result.angle = calculateAngle(points[0], points[1], points[2], points[3]);
  }

  // Calculate area and perimeter based on figure type
  switch (type) {
  case FigureType::Triangle:
    if (points.size() >= 3) {
      // Calculate triangle area and perimeter
      glm::vec2 v1 = points[1] - points[0];
      glm::vec2 v2 = points[2] - points[0];
      float cross = v1.x * v2.y - v1.y * v2.x;
      result.area = 0.5f * std::abs(cross);

      float perimeter = glm::distance(points[0], points[1]) +
                        glm::distance(points[1], points[2]) +
                        glm::distance(points[2], points[0]);
      result.perimeter = perimeter;
    }
    break;

  case FigureType::Quad:
    if (points.size() >= 4) {
      // Calculate quad area (split into 2 triangles)
      glm::vec2 v1a = points[1] - points[0];
      glm::vec2 v2a = points[2] - points[0];
      float cross1 = v1a.x * v2a.y - v1a.y * v2a.x;

      glm::vec2 v1b = points[2] - points[0];
      glm::vec2 v2b = points[3] - points[0];
      float cross2 = v1b.x * v2b.y - v1b.y * v2b.x;

      result.area = 0.5f * (std::abs(cross1) + std::abs(cross2));

      // Calculate perimeter
      float perimeter = glm::distance(points[0], points[1]) +
                        glm::distance(points[1], points[2]) +
                        glm::distance(points[2], points[3]) +
                        glm::distance(points[3], points[0]);
      result.perimeter = perimeter;
    }
    break;

  case FigureType::Circle:
    if (points.size() >= 2) {
      // Calculate circle area and perimeter
      float radius = glm::distance(points[0], points[1]);
      result.area = glm::pi<float>() * radius * radius;
      result.perimeter = 2.0f * glm::pi<float>() * radius;
    }
    break;

  case FigureType::Ngon:
    if (points.size() >= 3) {
      size_t n = points.size();
      float sideLength = glm::distance(points[0], points[1]);

      // Area of regular polygon
      float angle = glm::pi<float>() / static_cast<float>(n);
      result.area = (static_cast<float>(n) * sideLength * sideLength) /
                    (4.0f * glm::tan(angle));

      // Perimeter
      result.perimeter = static_cast<float>(n) * sideLength;
    }
    break;

  case FigureType::Line:
  case FigureType::Unknown:
  default:
    // For lines and unknown types, only distance is relevant
    break;
  }

  return result;
}

} // namespace view
