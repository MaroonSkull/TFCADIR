/**
 * @file DiameterDimensionTool.cpp
 * @brief Implementation of diameter dimension tool for Phase 11
 *
 * This file implements the DiameterDimensionTool class which creates diameter
 * dimensions showing the diameter of circles and arcs.
 */

#include "DimensionTool.hpp"
#include "View/UIFSMAdapter.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace view {

//------------------------------------------------------------------------------
// DiameterDimensionTool Implementation
//------------------------------------------------------------------------------

DiameterDimensionTool::DiameterDimensionTool(UIFSMAdapter& adapter)
    : DimensionTool(Type::DiameterDimension, adapter), currentMousePos_(0.0f) {
  /// Set default prefix for diameter dimensions
  style_.prefix = "Ø";
}

bool DiameterDimensionTool::handleClick(const glm::vec2& position,
                                        int modifiers) {
  /// Add the clicked point
  points_.push_back(position);

  const char* pointNames[] = {"Center", "Point on circle"};
  spdlog::info("DiameterDimensionTool: {} added at ({}, {})",
               pointNames[std::min(points_.size() - 1, size_t(1))], position.x,
               position.y);

  /// Check if we have enough points
  if (points_.size() >= 2) {
    /// Dimension is complete
    std::string value = getFormattedValue();
    spdlog::info("DiameterDimensionTool: Dimension complete - {}", value);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(value);
    }

    /// Clear points for next dimension
    points_.clear();
  }

  return true;
}

bool DiameterDimensionTool::handleMouseMove(const glm::vec2& position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool DiameterDimensionTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  return false;
}

void DiameterDimensionTool::cancel() {
  points_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  spdlog::info("DiameterDimensionTool: Cancelled");
}

bool DiameterDimensionTool::hasPreview() const {
  return !points_.empty() && points_.size() < 2;
}

std::vector<glm::vec2> DiameterDimensionTool::getPreviewPoints() const {
  std::vector<glm::vec2> preview;

  /// Add all collected points
  for (const auto& point : points_) {
    preview.push_back(point);
  }

  /// Add current mouse position as next point for preview
  if (points_.size() < 2) {
    preview.push_back(currentMousePos_);
  }

  return preview;
}

size_t DiameterDimensionTool::getCollectedPointsCount() const {
  return points_.size();
}

bool DiameterDimensionTool::isComplete() const {
  return points_.size() >= 2;
}

std::string DiameterDimensionTool::getFormattedValue() const {
  if (points_.size() < 2) {
    return "";
  }

  float diameter = getDiameter();
  return formatValue(diameter);
}

float DiameterDimensionTool::getDiameter() const {
  if (points_.size() < 2) {
    return 0.0f;
  }

  /// Calculate diameter (2 * radius)
  return 2.0f * glm::length(points_[1] - points_[0]);
}

glm::vec2 DiameterDimensionTool::getCenter() const {
  return points_.empty() ? glm::vec2(0.0f) : points_[0];
}

glm::vec2 DiameterDimensionTool::getPointOnCircle() const {
  return points_.size() < 2 ? glm::vec2(0.0f) : points_[1];
}

std::string DiameterDimensionTool::formatValue(float value) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << value << style_.suffix;
  return oss.str();
}

} // namespace view
