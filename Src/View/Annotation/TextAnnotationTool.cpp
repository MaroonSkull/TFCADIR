/**
 * @file TextAnnotationTool.cpp
 * @brief Implementation of text annotation tool for Phase 11
 *
 * This file implements the TextAnnotationTool class which creates text
 * labels and annotations on the canvas.
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
// TextAnnotationTool Implementation
//------------------------------------------------------------------------------

TextAnnotationTool::TextAnnotationTool(UIFSMAdapter &adapter)
    : DimensionTool(Type::TextAnnotation, adapter), position_(0.0f),
      hasPosition_(false), currentMousePos_(0.0f), text_(DEFAULT_TEXT),
      fontSize_(12.0f) {
  /// Text annotations don't use prefix/suffix
  style_.prefix = "";
  style_.suffix = "";
  style_.textSize = fontSize_;
}

bool TextAnnotationTool::handleClick(const glm::vec2 &position, int modifiers) {
  /// Set the text position
  position_ = position;
  hasPosition_ = true;

  spdlog::info("TextAnnotationTool: Position set at ({}, {})", position.x,
               position.y);
  spdlog::info("TextAnnotationTool: Annotation complete with text: '{}'",
               text_);

  /// Notify completion callback
  if (onComplete_) {
    onComplete_(text_);
  }

  /// Reset for next annotation
  hasPosition_ = false;

  return true;
}

bool TextAnnotationTool::handleMouseMove(const glm::vec2 &position) {
  /// Update current mouse position for preview
  currentMousePos_ = position;
  return true;
}

bool TextAnnotationTool::handleKeyPress(int key, int modifiers) {
  /// Handle Escape to cancel
  if (key == 256) { // GLFW_KEY_ESCAPE
    cancel();
    return true;
  }

  return false;
}

void TextAnnotationTool::cancel() {
  position_ = glm::vec2(0.0f);
  hasPosition_ = false;
  currentMousePos_ = glm::vec2(0.0f);
  text_ = DEFAULT_TEXT;
  spdlog::info("TextAnnotationTool: Cancelled");
}

bool TextAnnotationTool::hasPreview() const { return !hasPosition_; }

std::vector<glm::vec2> TextAnnotationTool::getPreviewPoints() const {
  std::vector<glm::vec2> preview;

  /// Add current mouse position as preview
  preview.push_back(currentMousePos_);

  return preview;
}

glm::vec2 TextAnnotationTool::getPosition() const {
  return hasPosition_ ? position_ : glm::vec2(0.0f);
}

void TextAnnotationTool::setText(const std::string &text) { text_ = text; }

void TextAnnotationTool::setFontSize(float size) { fontSize_ = size; }

size_t TextAnnotationTool::getCollectedPointsCount() const {
  return hasPosition_ ? 1 : 0;
}

bool TextAnnotationTool::isComplete() const { return hasPosition_; }

std::string TextAnnotationTool::getFormattedValue() const { return text_; }

} // namespace view
