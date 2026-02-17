/**
 * @file RadialDimensionTool.cpp
 * @brief Implementation of radial dimension tool for Phase 11
 *
 * This file implements the RadialDimensionTool class which creates radial
 * dimensions showing the radius of circles and arcs.
 */

#include "DimensionTool.hpp"
#include "View/UIFSMAdapter.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

//------------------------------------------------------------------------------
// RadialDimensionTool Implementation
//------------------------------------------------------------------------------

RadialDimensionTool::RadialDimensionTool(UIFSMAdapter &adapter)
    : DimensionTool(Type::RadialDimension, adapter), currentMousePos_(0.0f) {
  /// Set default prefix for radial dimensions
  style_.prefix = "R";
}

bool RadialDimensionTool::handleClick(const glm::vec2 &position,
                                      int modifiers) {
  /// Add the clicked point
  points_.push_back(position);

  const char *pointNames[] = {"Center", "Point on circle"};
  spdlog::info("RadialDimensionTool: {} added at ({}, {})",
               pointNames[std::min(points_.size() - 1, size_t(1))], position.x,
               position.y);

  /// Check if we have enough points
  if (points_.size() >= 2) {
    /// Dimension is complete
    std::string value = getFormattedValue();
    spdlog::info("RadialDimensionTool: Dimension complete - {}", value);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(value);
    }

    /// Clear points for next dimension
    points_.clear();
  }

  return true;
}

bool RadialDimensionTool::handleMouseMove(const glm::vec2 &position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool RadialDimensionTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  return false;
}

void RadialDimensionTool::cancel() {
  points_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  spdlog::info("RadialDimensionTool: Cancelled");
}

bool RadialDimensionTool::hasPreview() const {
  return !points_.empty() && points_.size() < 2;
}

std::vector<glm::vec2> RadialDimensionTool::getPreviewPoints() const {
  std::vector<glm::vec2> preview;

  /// Add all collected points
  for (const auto &point : points_) {
    preview.push_back(point);
  }

  /// Add current mouse position as next point for preview
  if (points_.size() < 2) {
    preview.push_back(currentMousePos_);
  }

  return preview;
}

size_t RadialDimensionTool::getCollectedPointsCount() const {
  return points_.size();
}

bool RadialDimensionTool::isComplete() const { return points_.size() >= 2; }

std::string RadialDimensionTool::getFormattedValue() const {
  if (points_.size() < 2) {
    return "";
  }

  float radius = getRadius();
  return formatValue(radius);
}

float RadialDimensionTool::getRadius() const {
  if (points_.size() < 2) {
    return 0.0f;
  }

  /// Calculate distance from center to point on circle
  return glm::length(points_[1] - points_[0]);
}

glm::vec2 RadialDimensionTool::getCenter() const {
  return points_.empty() ? glm::vec2(0.0f) : points_[0];
}

glm::vec2 RadialDimensionTool::getPointOnCircle() const {
  return points_.size() < 2 ? glm::vec2(0.0f) : points_[1];
}

std::string RadialDimensionTool::formatValue(float value) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << value << style_.suffix;
  return oss.str();
}

} // namespace view
