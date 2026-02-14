#include <View/Precision/CoordinateInputManager.hpp>
#include <View/Precision/CoordinateInputWidget.hpp>
#include <cstring>
#include <spdlog/spdlog.h>

namespace view {

CoordinateInputWidget::CoordinateInputWidget(UIFSMAdapter *uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter) {
  if (!uiFSMAdapter_) {
    throw std::invalid_argument("UIFSMAdapter cannot be null");
  }
  // Initialize input buffers with empty strings
  std::memset(bufferX_, 0, sizeof(bufferX_));
  std::memset(bufferY_, 0, sizeof(bufferY_));
  std::memset(bufferZ_, 0, sizeof(bufferZ_));
}

void CoordinateInputWidget::render() {
  // Query panel visibility from UIFSMAdapter (stateless coordinator pattern)
  bool visible = uiFSMAdapter_->getCoordinateInputWidgetVisible();
  if (!ImGui::Begin("Coordinate Input", &visible)) {
    ImGui::End();
    // Update visibility state in UIFSMAdapter if changed by user
    if (visible != uiFSMAdapter_->getCoordinateInputWidgetVisible()) {
      uiFSMAdapter_->setCoordinateInputWidgetVisible(visible);
    }
    return;
  }

  renderModeToggle();
  ImGui::Separator();
  renderCoordinateInputs();
  ImGui::Separator();
  renderCoordinatePreview();
  ImGui::Separator();
  renderButtons();

  ImGui::End();

  // Update visibility state in UIFSMAdapter if changed by user
  if (visible != uiFSMAdapter_->getCoordinateInputWidgetVisible()) {
    uiFSMAdapter_->setCoordinateInputWidgetVisible(visible);
  }
}

void CoordinateInputWidget::renderModeToggle() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  CoordinateInputSettings settings =
      uiFSMAdapter_->getCoordinateInputSettings();

  // Mode toggle radio buttons
  int mode = static_cast<int>(settings.inputMode);
  if (ImGui::RadioButton("Absolute", mode == 0)) {
    settings.inputMode = CoordinateInputMode::Absolute;
    uiFSMAdapter_->setCoordinateInputSettings(settings);
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Relative", mode == 1)) {
    settings.inputMode = CoordinateInputMode::Relative;
    uiFSMAdapter_->setCoordinateInputSettings(settings);
  }

  // Expression parsing toggle
  bool expressionParsing = settings.expressionParsingEnabled;
  if (ImGui::Checkbox("Enable Expressions", &expressionParsing)) {
    settings.expressionParsingEnabled = expressionParsing;
    uiFSMAdapter_->setCoordinateInputSettings(settings);
  }

  // Help tooltip for expression support
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::Text("Supports: +, -, *, /, parentheses");
    ImGui::Text("Functions: sin, cos, tan, sqrt, abs");
    ImGui::Text("Constants: pi, e");
    ImGui::EndTooltip();
  }
}

void CoordinateInputWidget::renderCoordinateInputs() {
  // X coordinate input
  ImGui::Text("X:");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::InputText("##X", bufferX_, sizeof(bufferX_));

  // Y coordinate input
  ImGui::Text("Y:");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::InputText("##Y", bufferY_, sizeof(bufferY_));

  // Z coordinate input
  ImGui::Text("Z:");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::InputText("##Z", bufferZ_, sizeof(bufferZ_));
}

void CoordinateInputWidget::renderCoordinatePreview() {
  ImGui::Text("Parsed Preview:");

  // Query current settings for precision
  CoordinateInputSettings settings =
      uiFSMAdapter_->getCoordinateInputSettings();
  int precision = settings.precision;

  // Create a temporary CoordinateInputManager for parsing
  // Note: In a production environment, this would be injected or shared
  CoordinateInputManager tempManager(*uiFSMAdapter_);

  // Parse and display X coordinate
  if (bufferX_[0] != '\0') {
    auto result = tempManager.parseExpression(std::string(bufferX_));
    if (result.isValid()) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "X: %.*f", precision,
                         result.value.value());
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "X: Error - %s",
                         result.errorMessage.c_str());
    }
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "X: --");
  }

  // Parse and display Y coordinate
  if (bufferY_[0] != '\0') {
    auto result = tempManager.parseExpression(std::string(bufferY_));
    if (result.isValid()) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Y: %.*f", precision,
                         result.value.value());
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Y: Error - %s",
                         result.errorMessage.c_str());
    }
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Y: --");
  }

  // Parse and display Z coordinate
  if (bufferZ_[0] != '\0') {
    auto result = tempManager.parseExpression(std::string(bufferZ_));
    if (result.isValid()) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Z: %.*f", precision,
                         result.value.value());
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Z: Error - %s",
                         result.errorMessage.c_str());
    }
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Z: --");
  }
}

void CoordinateInputWidget::renderButtons() {
  // Apply button - submit coordinates
  if (ImGui::Button("Apply", ImVec2(-FLT_MIN / 3.0f, 0.0f))) {
    // Create a temporary CoordinateInputManager for parsing
    CoordinateInputManager tempManager(*uiFSMAdapter_);

    // Parse all coordinates
    auto resultX = tempManager.parseExpression(std::string(bufferX_));
    auto resultY = tempManager.parseExpression(std::string(bufferY_));
    auto resultZ = tempManager.parseExpression(std::string(bufferZ_));

    // Check if all coordinates are valid
    if (resultX.isValid() && resultY.isValid() && resultZ.isValid()) {
      // Create glm::vec3 from parsed coordinates
      glm::vec3 submittedCoordinate(
          resultX.value.value(), resultY.value.value(), resultZ.value.value());

      // Log the parsed coordinates for debugging and audit trail
      spdlog::info("Coordinate Input Submitted: X={}, Y={}, Z={}",
                   submittedCoordinate.x, submittedCoordinate.y,
                   submittedCoordinate.z);

      // Note: Full FSM event integration for coordinate submission is
      // pending. The coordinates are logged and can be processed by
      // other components. Future implementation should:
      // 1. Add setCoordinateInputCoordinates() method to UIFSMAdapter
      // 2. Define OnCoordinateSubmitted event in FSM configuration
      // 3. Trigger the event via uiFSMAdapter_->fsm_.process_event()

      // Clear buffers after successful submission
      std::memset(bufferX_, 0, sizeof(bufferX_));
      std::memset(bufferY_, 0, sizeof(bufferY_));
      std::memset(bufferZ_, 0, sizeof(bufferZ_));
    } else {
      // Log parsing errors
      std::string errors;
      if (!resultX.isValid()) {
        errors += "X: " + resultX.errorMessage + "; ";
      }
      if (!resultY.isValid()) {
        errors += "Y: " + resultY.errorMessage + "; ";
      }
      if (!resultZ.isValid()) {
        errors += "Z: " + resultZ.errorMessage + "; ";
      }
      spdlog::warn("Coordinate Input Parse Errors: {}", errors);
    }
  }

  ImGui::SameLine();

  // Clear button - reset input buffers
  if (ImGui::Button("Clear", ImVec2(-FLT_MIN / 3.0f, 0.0f))) {
    std::memset(bufferX_, 0, sizeof(bufferX_));
    std::memset(bufferY_, 0, sizeof(bufferY_));
    std::memset(bufferZ_, 0, sizeof(bufferZ_));
  }

  ImGui::SameLine();

  // Cancel button - hide the widget
  if (ImGui::Button("Cancel", ImVec2(-FLT_MIN / 3.0f, 0.0f))) {
    // Clear buffers
    std::memset(bufferX_, 0, sizeof(bufferX_));
    std::memset(bufferY_, 0, sizeof(bufferY_));
    std::memset(bufferZ_, 0, sizeof(bufferZ_));

    // Hide the widget
    uiFSMAdapter_->setCoordinateInputWidgetVisible(false);
  }
}

} // namespace view
