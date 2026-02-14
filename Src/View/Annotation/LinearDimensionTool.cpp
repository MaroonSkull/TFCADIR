/**
 * @file LinearDimensionTool.cpp
 * @brief Implementation of linear dimension tool for Phase 11
 *
 * This file implements the LinearDimensionTool class which creates linear
 * dimensions showing the distance between two points. The tool follows the
 * SolidWorks-inspired conventional CAD design pattern.
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
// LinearDimensionTool Implementation
//------------------------------------------------------------------------------

LinearDimensionTool::LinearDimensionTool(UIFSMAdapter &adapter)
    : DimensionTool(Type::LinearDimension, adapter), currentMousePos_(0.0f) {}

bool LinearDimensionTool::handleClick(const glm::vec2 &position,
                                      int modifiers) {
  /// Add the clicked point
  points_.push_back(position);
  spdlog::info("LinearDimensionTool: Point {} added at ({}, {})",
               points_.size(), position.x, position.y);

  /// Check if we have enough points
  if (points_.size() >= 2) {
    /// Dimension is complete
    std::string value = getFormattedValue();
    spdlog::info("LinearDimensionTool: Dimension complete - {}", value);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(value);
    }

    /// Clear points for next dimension
    points_.clear();
  }

  return true;
}

bool LinearDimensionTool::handleMouseMove(const glm::vec2 &position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool LinearDimensionTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  /// Handle Tab to cycle orientation
  if (key == 258) { // GLFW_KEY_TAB
    switch (orientation_) {
    case Orientation::Aligned:
      orientation_ = Orientation::Horizontal;
      spdlog::info("LinearDimensionTool: Orientation changed to Horizontal");
      break;
    case Orientation::Horizontal:
      orientation_ = Orientation::Vertical;
      spdlog::info("LinearDimensionTool: Orientation changed to Vertical");
      break;
    case Orientation::Vertical:
      orientation_ = Orientation::Aligned;
      spdlog::info("LinearDimensionTool: Orientation changed to Aligned");
      break;
    }
    return true;
  }

  return false;
}

void LinearDimensionTool::cancel() {
  points_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  spdlog::info("LinearDimensionTool: Cancelled");
}

bool LinearDimensionTool::hasPreview() const {
  return !points_.empty() && points_.size() < 2;
}

std::vector<glm::vec2> LinearDimensionTool::getPreviewPoints() const {
  std::vector<glm::vec2> preview;

  if (points_.empty()) {
    return preview;
  }

  /// Add the first point
  preview.push_back(points_[0]);

  /// Add current mouse position as second point for preview
  if (points_.size() == 1) {
    preview.push_back(currentMousePos_);
  }

  return preview;
}

size_t LinearDimensionTool::getCollectedPointsCount() const {
  return points_.size();
}

bool LinearDimensionTool::isComplete() const { return points_.size() >= 2; }

std::string LinearDimensionTool::getFormattedValue() const {
  if (points_.size() < 2) {
    return "";
  }

  float distance = getDistance();
  return formatValue(distance);
}

float LinearDimensionTool::getDistance() const {
  if (points_.size() < 2) {
    return 0.0f;
  }

  glm::vec2 start = points_[0];
  glm::vec2 end = points_[1];

  /// Calculate distance based on orientation
  switch (orientation_) {
  case Orientation::Horizontal:
    return std::abs(end.x - start.x);
  case Orientation::Vertical:
    return std::abs(end.y - start.y);
  case Orientation::Aligned:
  default:
    return glm::length(end - start);
  }
}

float LinearDimensionTool::getAngle() const {
  if (points_.size() < 2) {
    return 0.0f;
  }

  glm::vec2 delta = points_[1] - points_[0];
  return std::atan2(delta.y, delta.x);
}

glm::vec2 LinearDimensionTool::getStartPoint() const {
  return points_.empty() ? glm::vec2(0.0f) : points_[0];
}

glm::vec2 LinearDimensionTool::getEndPoint() const {
  return points_.size() < 2 ? glm::vec2(0.0f) : points_[1];
}

void LinearDimensionTool::setOrientation(Orientation orientation) {
  orientation_ = orientation;
}

LinearDimensionTool::Orientation LinearDimensionTool::getOrientation() const {
  return orientation_;
}

std::pair<glm::vec2, glm::vec2>
LinearDimensionTool::calculateDimensionLine(const glm::vec2 &start,
                                            const glm::vec2 &end,
                                            const glm::vec2 &mousePos) const {
  /// Calculate the dimension line based on orientation
  glm::vec2 dimStart, dimEnd;

  switch (orientation_) {
  case Orientation::Horizontal: {
    /// Horizontal dimension line
    float y = mousePos.y;
    dimStart = glm::vec2(start.x, y);
    dimEnd = glm::vec2(end.x, y);
    break;
  }
  case Orientation::Vertical: {
    /// Vertical dimension line
    float x = mousePos.x;
    dimStart = glm::vec2(x, start.y);
    dimEnd = glm::vec2(x, end.y);
    break;
  }
  case Orientation::Aligned:
  default: {
    /// Aligned dimension line - parallel to measured line
    glm::vec2 delta = end - start;
    glm::vec2 normal = glm::normalize(glm::vec2(-delta.y, delta.x));

    /// Calculate offset from measured line based on mouse position
    glm::vec2 midPoint = (start + end) * 0.5f;
    glm::vec2 toMouse = mousePos - midPoint;
    float offset = glm::dot(toMouse, normal);

    dimStart = start + normal * offset;
    dimEnd = end + normal * offset;
    break;
  }
  }

  return {dimStart, dimEnd};
}

std::string LinearDimensionTool::formatValue(float value) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(style_.precision);
  oss << style_.prefix << value << style_.suffix;
  return oss.str();
}

//------------------------------------------------------------------------------
// DimensionTool Base Implementation
//------------------------------------------------------------------------------

DimensionTool::DimensionTool(Type type, UIFSMAdapter &adapter)
    : type_(type), adapter_(adapter) {}

std::string DimensionTool::getToolId() const { return typeToString(type_); }

void DimensionTool::activate() {
  /// Trigger FSM event for tool activation
  adapter_.activateTool(getToolId());
}

void DimensionTool::deactivate() {
  /// Trigger FSM event for tool deactivation
  adapter_.deactivateTool();
}

bool DimensionTool::isActive() const {
  /// Check if this tool is currently active
  return adapter_.getActiveTool() == getToolId();
}

std::string DimensionTool::typeToString(Type type) {
  switch (type) {
  case Type::LinearDimension:
    return "LinearDimension";
  case Type::AngularDimension:
    return "AngularDimension";
  case Type::RadialDimension:
    return "RadialDimension";
  case Type::DiameterDimension:
    return "DiameterDimension";
  case Type::LeaderLine:
    return "LeaderLine";
  case Type::TextAnnotation:
    return "TextAnnotation";
  default:
    return "Unknown";
  }
}

//------------------------------------------------------------------------------
// DimensionToolFactory Implementation
//------------------------------------------------------------------------------

std::unique_ptr<DimensionTool>
DimensionToolFactory::createTool(DimensionTool::Type type,
                                 UIFSMAdapter &adapter) {
  switch (type) {
  case DimensionTool::Type::LinearDimension:
    return std::make_unique<LinearDimensionTool>(adapter);
  case DimensionTool::Type::AngularDimension:
    return std::make_unique<AngularDimensionTool>(adapter);
  case DimensionTool::Type::RadialDimension:
    return std::make_unique<RadialDimensionTool>(adapter);
  case DimensionTool::Type::DiameterDimension:
    return std::make_unique<DiameterDimensionTool>(adapter);
  case DimensionTool::Type::LeaderLine:
    return std::make_unique<LeaderTool>(adapter);
  case DimensionTool::Type::TextAnnotation:
    return std::make_unique<TextAnnotationTool>(adapter);
  default:
    spdlog::warn("DimensionToolFactory: Unknown tool type {}",
                 static_cast<int>(type));
    return nullptr;
  }
}

std::vector<DimensionTool::Type> DimensionToolFactory::getAvailableTools() {
  return {DimensionTool::Type::LinearDimension,
          DimensionTool::Type::AngularDimension,
          DimensionTool::Type::RadialDimension,
          DimensionTool::Type::DiameterDimension,
          DimensionTool::Type::LeaderLine,
          DimensionTool::Type::TextAnnotation};
}

std::string DimensionToolFactory::getToolDisplayName(DimensionTool::Type type) {
  switch (type) {
  case DimensionTool::Type::LinearDimension:
    return "Linear Dimension";
  case DimensionTool::Type::AngularDimension:
    return "Angular Dimension";
  case DimensionTool::Type::RadialDimension:
    return "Radial Dimension";
  case DimensionTool::Type::DiameterDimension:
    return "Diameter Dimension";
  case DimensionTool::Type::LeaderLine:
    return "Leader Line";
  case DimensionTool::Type::TextAnnotation:
    return "Text Annotation";
  default:
    return "Unknown Tool";
  }
}

} // namespace view
