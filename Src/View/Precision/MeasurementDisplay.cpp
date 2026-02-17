#include "View/Precision/MeasurementDisplay.hpp"
#include <stdexcept>
#include <string>

namespace view {

// ==========================================================================
// Constructor
// ==========================================================================

MeasurementDisplay::MeasurementDisplay(UIFSMAdapter *uiFSMAdapter,
                                       MeasurementManager *measurementManager)
    : uiFSMAdapter_(uiFSMAdapter), measurementManager_(measurementManager) {

  if (uiFSMAdapter_ == nullptr) {
    throw std::invalid_argument("uiFSMAdapter cannot be null");
  }

  if (measurementManager_ == nullptr) {
    throw std::invalid_argument("measurementManager cannot be null");
  }
}

// ==========================================================================
// Public Methods
// ==========================================================================

void MeasurementDisplay::render() {
  /// Query visibility from UIFSMAdapter (stateless coordinator pattern)
  if (!uiFSMAdapter_->getMeasurementDisplayVisible()) {
    return;
  }

  // Get the background draw list for overlay rendering
  ImDrawList *drawList = ImGui::GetBackgroundDrawList();

  if (drawList == nullptr) {
    return;
  }

  // Get measurement settings for precision
  MeasurementSettings settings = uiFSMAdapter_->getMeasurementSettings();
  int precision = settings.precision;

  // Calculate display position
  ImVec2 displayPos = position_;
  ImVec2 currentPos = displayPos;

  // Build the content string to calculate box size
  std::string content;
  int lineCount = 0;

  if (currentData_.isValid) {
    if (currentData_.distance.has_value() && settings.showDistance) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Distance: %.*f", precision,
               currentData_.distance.value());
      content += buffer;
      content += "\n";
      lineCount++;
    }

    if (currentData_.angle.has_value() && settings.showAngle) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Angle: %.*f°", precision,
               currentData_.angle.value());
      content += buffer;
      content += "\n";
      lineCount++;
    }

    if (currentData_.area.has_value() && settings.showArea) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Area: %.*f", precision,
               currentData_.area.value());
      content += buffer;
      content += "\n";
      lineCount++;
    }

    if (currentData_.perimeter.has_value() && settings.showPerimeter) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Perimeter: %.*f", precision,
               currentData_.perimeter.value());
      content += buffer;
      content += "\n";
      lineCount++;
    }

    if (settings.showDistance || settings.showAngle || settings.showArea ||
        settings.showPerimeter) {
      content += "\n"; // Separator between measurements and coordinates
      lineCount++;
    }

    if (showCoordinates_) {
      char buffer[128];
      snprintf(buffer, sizeof(buffer), "Start: (%.2f, %.2f, %.2f)",
               currentData_.startPoint.x, currentData_.startPoint.y,
               currentData_.startPoint.z);
      content += buffer;
      content += "\n";
      lineCount++;

      snprintf(buffer, sizeof(buffer), "End: (%.2f, %.2f, %.2f)",
               currentData_.endPoint.x, currentData_.endPoint.y,
               currentData_.endPoint.z);
      content += buffer;
      content += "\n";
      lineCount++;

      snprintf(buffer, sizeof(buffer), "Cursor: (%.2f, %.2f, %.2f)",
               currentData_.cursorPosition.x, currentData_.cursorPosition.y,
               currentData_.cursorPosition.z);
      content += buffer;
      lineCount++;
    }
  }

  // Don't render if no content
  if (content.empty() || lineCount == 0) {
    return;
  }

  // Calculate approximate text size
  float lineHeight = fontSize_ + padding_.y;
  float boxWidth = 200.0f; // Approximate width
  float boxHeight = lineHeight * lineCount + padding_.y * 2;

  // Draw semi-transparent background
  ImVec2 boxMin = currentPos;
  ImVec2 boxMax = ImVec2(currentPos.x + boxWidth, currentPos.y + boxHeight);
  drawList->AddRectFilled(boxMin, boxMax, COLOR_BACKGROUND, 5.0f);

  // Draw border
  drawList->AddRect(boxMin, boxMax, ImColor(1.0f, 1.0f, 1.0f, 0.3f), 5.0f, 0.0f,
                    1.0f);

  // Update position for text (with padding)
  currentPos.x += padding_.x;
  currentPos.y += padding_.y;

  // Render measurements with colors
  if (currentData_.isValid) {
    if (currentData_.distance.has_value() && settings.showDistance) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Distance: %.*f", precision,
               currentData_.distance.value());
      renderMeasurementLine(drawList, currentPos, "Distance: ", buffer + 10,
                            COLOR_DISTANCE);
    }

    if (currentData_.angle.has_value() && settings.showAngle) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Angle: %.*f°", precision,
               currentData_.angle.value());
      renderMeasurementLine(drawList, currentPos, "Angle: ", buffer + 7,
                            COLOR_ANGLE);
    }

    if (currentData_.area.has_value() && settings.showArea) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Area: %.*f", precision,
               currentData_.area.value());
      renderMeasurementLine(drawList, currentPos, "Area: ", buffer + 6,
                            COLOR_AREA);
    }

    if (currentData_.perimeter.has_value() && settings.showPerimeter) {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Perimeter: %.*f", precision,
               currentData_.perimeter.value());
      renderMeasurementLine(drawList, currentPos, "Perimeter: ", buffer + 11,
                            COLOR_PERIMETER);
    }

    // Add spacing before coordinates
    if (showCoordinates_ && (settings.showDistance || settings.showAngle ||
                             settings.showArea || settings.showPerimeter)) {
      currentPos.y += lineHeight * 0.5f;
    }

    // Render coordinates
    if (showCoordinates_) {
      renderCoordinate(drawList, currentPos,
                       "Start: ", currentData_.startPoint);
      renderCoordinate(drawList, currentPos, "End: ", currentData_.endPoint);
      renderCoordinate(drawList, currentPos,
                       "Cursor: ", currentData_.cursorPosition);
    }
  }
}

void MeasurementDisplay::setMeasurementData(const MeasurementData &data) {
  currentData_ = data;
}

const MeasurementData &MeasurementDisplay::getMeasurementData() const {
  return currentData_;
}

// ==========================================================================
// Private Helper Methods
// ==========================================================================

void MeasurementDisplay::renderMeasurementLine(ImDrawList *drawList,
                                               ImVec2 &position,
                                               const char *label,
                                               const char *value,
                                               ImU32 color) const {
  // Render label in white
  const char *labelEnd = label;
  while (*labelEnd != '\0' && *labelEnd != ':') {
    labelEnd++;
  }
  labelEnd++; // Include the colon

  drawList->AddText(position, COLOR_COORDINATES, label, labelEnd);

  // Calculate label width
  ImVec2 labelSize = ImGui::CalcTextSize(label, labelEnd);
  position.x += labelSize.x;

  // Render value in specified color
  drawList->AddText(position, color, value);

  // Move to next line
  position.x = position_.x + padding_.x;
  position.y += fontSize_ + padding_.y;
}

void MeasurementDisplay::renderCoordinate(ImDrawList *drawList,
                                          ImVec2 &position, const char *label,
                                          const glm::vec3 &coord) const {
  // Get precision from settings
  MeasurementSettings settings = uiFSMAdapter_->getMeasurementSettings();
  int precision = settings.precision;

  // Format coordinate string
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%s%.*f, %.*f, %.*f)", label, precision,
           coord.x, precision, coord.y, precision, coord.z);

  // Render in white
  drawList->AddText(position, COLOR_COORDINATES, buffer);

  // Move to next line
  position.x = position_.x + padding_.x;
  position.y += fontSize_ + padding_.y;
}

} // namespace view
