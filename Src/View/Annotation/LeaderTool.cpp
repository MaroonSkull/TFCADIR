/**
 * @file LeaderTool.cpp
 * @brief Implementation of leader line tool for Phase 11
 *
 * This file implements the LeaderTool class which creates leader lines
 * with text annotations pointing to specific locations on the canvas.
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
// LeaderTool Implementation
//------------------------------------------------------------------------------

LeaderTool::LeaderTool(UIFSMAdapter& adapter)
    : DimensionTool(Type::LeaderLine, adapter), currentMousePos_(0.0f),
      text_(DEFAULT_TEXT) {
  /// Leader lines don't use prefix/suffix
  style_.prefix = "";
  style_.suffix = "";
}

bool LeaderTool::handleClick(const glm::vec2& position, int modifiers) {
  /// Add the clicked point
  points_.push_back(position);

  const char* pointNames[] = {"Arrow point", "Text position"};
  spdlog::info("LeaderTool: {} added at ({}, {})",
               pointNames[std::min(points_.size() - 1, size_t(1))], position.x,
               position.y);

  /// Check if we have enough points
  if (points_.size() >= 2) {
    /// Leader is complete
    spdlog::info("LeaderTool: Leader complete with text: '{}'", text_);

    /// Notify completion callback
    if (onComplete_) {
      onComplete_(text_);
    }

    /// Clear points for next leader
    points_.clear();
  }

  return true;
}

bool LeaderTool::handleMouseMove(const glm::vec2& position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool LeaderTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  return false;
}

void LeaderTool::cancel() {
  points_.clear();
  currentMousePos_ = glm::vec2(0.0f);
  text_ = DEFAULT_TEXT;
  spdlog::info("LeaderTool: Cancelled");
}

bool LeaderTool::hasPreview() const {
  return !points_.empty() && points_.size() < 2;
}

std::vector<glm::vec2> LeaderTool::getPreviewPoints() const {
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

glm::vec2 LeaderTool::getArrowPoint() const {
  return points_.empty() ? glm::vec2(0.0f) : points_[0];
}

glm::vec2 LeaderTool::getTextPosition() const {
  return points_.size() < 2 ? glm::vec2(0.0f) : points_[1];
}

void LeaderTool::setText(const std::string& text) { text_ = text; }

float LeaderTool::getLength() const {
  if (points_.size() < 2) {
    return 0.0f;
  }

  return glm::length(points_[1] - points_[0]);
}

size_t LeaderTool::getCollectedPointsCount() const {
  return points_.size();
}

bool LeaderTool::isComplete() const {
  return points_.size() >= 2;
}

std::string LeaderTool::getFormattedValue() const {
  return text_;
}

} // namespace view
