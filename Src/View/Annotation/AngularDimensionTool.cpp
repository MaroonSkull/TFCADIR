/**
 * @file AngularDimensionTool.cpp
 * @brief Implementation of angular dimension tool for Phase 11
 *
 * This file implements the AngularDimensionTool class which creates angular
 * dimensions showing the angle between two lines or three points.
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
// AngularDimensionTool Implementation
//------------------------------------------------------------------------------

AngularDimensionTool::AngularDimensionTool(UIFSMAdapter &adapter)
    : DimensionTool(Type::AngularDimension, adapter), currentMousePos_(0.0f) {
  /// Set default prefix for angular dimensions
  style_.suffix = "°";
}

bool AngularDimensionTool::handleClick(const glm::vec2 &position,
                                       int modifiers) {
  /// Add the clicked point
  points_.push_back(position);

  const char *pointNames[] = {"Vertex", "First point", "Second point"};
  spdlog::info("AngularDimensionTool: {} added at ({}, {})",
               pointNames[std::min(points_.size() - 1, size_t(2))], position.x,
               position.y);

  /// Check if we have enough points
  if (points_.size() >= 3) {
    /// Dimension is complete
    std::string value = getFormattedValue();
    spdlog::info("AngularDimensionTool: Dimension complete - {}", value);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(value);
    }

    /// Clear points for next dimension
    points_.clear();
  }

  return true;
}

bool AngularDimensionTool::handleMouseMove(const glm::vec2 &position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool AngularDimensionTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  return false;
}

void AngularDimensionTool::cancel() {
  points_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  spdlog::info("AngularDimensionTool: Cancelled");
}

bool AngularDimensionTool::hasPreview() const {
  return !points_.empty() && points_.size() < 3;
}

std::vector<glm::vec2> AngularDimensionTool::getPreviewPoints() const {
  std::vector<glm::vec2> preview;

  /// Add all collected points
  for (const auto &point : points_) {
    preview.push_back(point);
  }

  /// Add current mouse position as next point for preview
  if (points_.size() < 3) {
    preview.push_back(currentMousePos_);
  }

  return preview;
}

size_t AngularDimensionTool::getCollectedPointsCount() const {
  return points_.size();
}

bool AngularDimensionTool::isComplete() const { return points_.size() >= 3; }

std::string AngularDimensionTool::getFormattedValue() const {
  if (points_.size() < 3) {
    return "";
  }

  float angle = getAngle();
  return formatValue(angle);
}

float AngularDimensionTool::getAngle() const {
  if (points_.size() < 3) {
    return 0.0f;
  }

  /// Calculate vectors from vertex to each point
  glm::vec2 v1 = points_[1] - points_[0]; // First line direction
  glm::vec2 v2 = points_[2] - points_[0]; // Second line direction

  return calculateAngle(v1, v2);
}

glm::vec2 AngularDimensionTool::getVertex() const {
  return points_.empty() ? glm::vec2(0.0f) : points_[0];
}

glm::vec2 AngularDimensionTool::getFirstPoint() const {
  return points_.size() < 2 ? glm::vec2(0.0f) : points_[1];
}

glm::vec2 AngularDimensionTool::getSecondPoint() const {
  return points_.size() < 3 ? glm::vec2(0.0f) : points_[2];
}

float AngularDimensionTool::calculateAngle(const glm::vec2 &v1,
                                           const glm::vec2 &v2) const {
  /// Calculate angle using dot product
  float dot = glm::dot(v1, v2);
  float len1 = glm::length(v1);
  float len2 = glm::length(v2);

  if (len1 < 0.0001f || len2 < 0.0001f) {
    return 0.0f;
  }

  /// Clamp to avoid numerical issues with acos
  float cosAngle = glm::clamp(dot / (len1 * len2), -1.0f, 1.0f);

  /// Convert to degrees
  return glm::degrees(std::acos(cosAngle));
}

std::string AngularDimensionTool::formatValue(float value) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << value << style_.suffix;
  return oss.str();
}

} // namespace view
