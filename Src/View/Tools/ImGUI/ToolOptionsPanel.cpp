/**
 * @file ToolOptionsPanel.cpp
 * @brief Implementation of ToolOptionsPanel for Phase 2 drawing tools
 *
 * This file implements the ToolOptionsPanel class which provides a stateless
 * ImGUI interface for tool selection and options configuration. All state is
 * queried from the FSM via UIFSMAdapter each frame.
 */

#include "ToolOptionsPanel.hpp"
#include "UIFSMAdapter.hpp"
#include <imgui.h>
#include <map>
#include <string>

namespace view {

namespace ImGUI {

/// Define the list of available tools with their display names and events
const std::vector<ToolOptionsPanel::ToolInfo>
    ToolOptionsPanel::availableTools_ = {
        {"Line3D", "Line 3D", "OnActivateLine3D"},
        {"Circle3D", "Circle 3D", "OnActivateCircle3D"},
        {"Arc3D", "Arc 3D", "OnActivateArc3D"},
        {"Rectangle3D", "Rectangle 3D", "OnActivateRectangle3D"},
        {"Polygon3D", "Polygon 3D", "OnActivatePolygon3D"},
        {"NGon3D", "N-gon 3D", "OnActivateNGon3D"},
        {"LineInSketch", "Line in Sketch", "OnActivateLineInSketch"},
        {"CircleInSketch", "Circle in Sketch", "OnActivateCircleInSketch"},
};

ToolOptionsPanel::ToolOptionsPanel(UIFSMAdapter &adapter)
    : adapter_(adapter), onToolActivate_(nullptr), onOptionChange_(nullptr) {}

void ToolOptionsPanel::render(const ImVec2 &panelSize) {
  /// Query the FSM for the current active tool
  std::string activeTool = adapter_.getActiveTool();

  /// Query the FSM for collected points
  std::vector<glm::vec3> collectedPoints = adapter_.getCollectedPoints();

  /// Query the FSM for the current tool options
  ToolOptions options = adapter_.getToolOptions();

  /// Set the panel size
  ImGui::SetNextWindowSize(panelSize, ImGuiCond_FirstUseEver);

  /// Begin the tool options panel
  if (ImGui::Begin("Tool Options")) {
    /// Render tool selection section
    renderToolSelection(activeTool);

    /// Separator between sections
    ImGui::Separator();

    /// Render tool options section
    renderToolOptions(activeTool, options);

    /// Separator between sections
    ImGui::Separator();

    /// Render status section
    renderStatus(activeTool, collectedPoints.size());
  }
  ImGui::End();
}

void ToolOptionsPanel::renderToolSelection(const std::string &activeTool) {
  ImGui::Text("Tool Selection:");

  /// Render radio buttons for all available tools
  for (const auto &tool : availableTools_) {
    /// Check if this is the active tool
    bool isActive = (activeTool == tool.id);

    /// Render radio button
    if (ImGui::RadioButton(tool.displayName.c_str(), isActive)) {
      /// User clicked to activate this tool
      if (onToolActivate_) {
        onToolActivate_(tool.id);
      }
    }
  }
}

void ToolOptionsPanel::renderToolOptions(const std::string &activeTool,
                                         const ToolOptions &options) {
  ImGui::Text("Options:");

  /// Render tool-specific options based on the active tool
  if (activeTool == "Line3D") {
    /// Line3D options: creation method
    std::vector<std::string> methods = {"Continuous", "Segment"};
    std::string method = "Continuous";
    if (options.count("method") > 0) {
      if (options.at("method").type() == typeid(std::string)) {
        method = std::any_cast<std::string>(options.at("method"));
      }
    }
    renderEnumOption("Method", method, methods);
  } else if (activeTool == "Circle3D") {
    /// Circle3D options: radius, segments
    float radius = 1.0f;
    if (options.count("radius") > 0) {
      if (options.at("radius").type() == typeid(float)) {
        radius = std::any_cast<float>(options.at("radius"));
      }
    }
    renderFloatOption("Radius", radius, 0.1f, 100.0f);

    int segments = 32;
    if (options.count("segments") > 0) {
      if (options.at("segments").type() == typeid(int)) {
        segments = std::any_cast<int>(options.at("segments"));
      }
    }
    renderIntOption("Segments", segments, 3, 128);
  } else if (activeTool == "Arc3D") {
    /// Arc3D options: radius, start angle, end angle
    float radius = 1.0f;
    if (options.count("radius") > 0) {
      if (options.at("radius").type() == typeid(float)) {
        radius = std::any_cast<float>(options.at("radius"));
      }
    }
    renderFloatOption("Radius", radius, 0.1f, 100.0f);

    float startAngle = 0.0f;
    if (options.count("startAngle") > 0) {
      if (options.at("startAngle").type() == typeid(float)) {
        startAngle = std::any_cast<float>(options.at("startAngle"));
      }
    }
    renderFloatOption("Start Angle", startAngle, 0.0f, 360.0f);

    float endAngle = 180.0f;
    if (options.count("endAngle") > 0) {
      if (options.at("endAngle").type() == typeid(float)) {
        endAngle = std::any_cast<float>(options.at("endAngle"));
      }
    }
    renderFloatOption("End Angle", endAngle, 0.0f, 360.0f);
  } else if (activeTool == "Rectangle3D") {
    /// Rectangle3D options: corner type, fill
    std::vector<std::string> corners = {"First", "Second", "Center"};
    std::string corner = "First";
    if (options.count("corner") > 0) {
      if (options.at("corner").type() == typeid(std::string)) {
        corner = std::any_cast<std::string>(options.at("corner"));
      }
    }
    renderEnumOption("Corner", corner, corners);

    bool fill = false;
    if (options.count("fill") > 0) {
      if (options.at("fill").type() == typeid(bool)) {
        fill = std::any_cast<bool>(options.at("fill"));
      }
    }
    renderBoolOption("Fill", fill);
  } else if (activeTool == "Polygon3D" || activeTool == "NGon3D") {
    /// Polygon3D/NGon3D options: sides, radius, regular
    int sides = 5;
    if (options.count("sides") > 0) {
      if (options.at("sides").type() == typeid(int)) {
        sides = std::any_cast<int>(options.at("sides"));
      }
    }
    renderIntOption("Sides", sides, 3, 32);

    float radius = 1.0f;
    if (options.count("radius") > 0) {
      if (options.at("radius").type() == typeid(float)) {
        radius = std::any_cast<float>(options.at("radius"));
      }
    }
    renderFloatOption("Radius", radius, 0.1f, 100.0f);

    bool regular = true;
    if (options.count("regular") > 0) {
      if (options.at("regular").type() == typeid(bool)) {
        regular = std::any_cast<bool>(options.at("regular"));
      }
    }
    renderBoolOption("Regular", regular);
  } else if (activeTool == "LineInSketch" || activeTool == "CircleInSketch") {
    /// Sketch mode options
    ImGui::Text("Sketch mode options not yet implemented");
  }
}

void ToolOptionsPanel::renderStatus(const std::string &activeTool,
                                    size_t collectedPoints) {
  ImGui::Text("Status:");

  /// Display current tool
  if (!activeTool.empty()) {
    ImGui::Text("Tool: %s", activeTool.c_str());
  } else {
    ImGui::Text("Tool: None");
  }

  /// Display collected points count
  ImGui::Text("Points: %zu", collectedPoints);
}

void ToolOptionsPanel::renderFloatOption(const std::string &name, float value,
                                         float minValue, float maxValue) {
  /// Render a float input field with drag functionality
  float tempValue = value;
  if (ImGui::DragFloat(name.c_str(), &tempValue, 0.1f, minValue, maxValue,
                       "%.2f")) {
    /// Value changed, notify via callback
    if (onOptionChange_) {
      onOptionChange_(name, tempValue);
    }
  }
}

void ToolOptionsPanel::renderIntOption(const std::string &name, int value,
                                       int minValue, int maxValue) {
  /// Render an integer input field with drag functionality
  int tempValue = value;
  if (ImGui::DragInt(name.c_str(), &tempValue, 1.0f, minValue, maxValue)) {
    /// Value changed, notify via callback
    if (onOptionChange_) {
      onOptionChange_(name, tempValue);
    }
  }
}

void ToolOptionsPanel::renderBoolOption(const std::string &name, bool value) {
  /// Render a checkbox for boolean options
  bool tempValue = value;
  if (ImGui::Checkbox(name.c_str(), &tempValue)) {
    /// Value changed, notify via callback
    if (onOptionChange_) {
      onOptionChange_(name, tempValue);
    }
  }
}

void ToolOptionsPanel::renderEnumOption(
    const std::string &name, const std::string &value,
    const std::vector<std::string> &options) {
  /// Render a combo box (dropdown) for enum options
  int currentIndex = 0;
  for (size_t i = 0; i < options.size(); ++i) {
    if (options[i] == value) {
      currentIndex = static_cast<int>(i);
      break;
    }
  }

  /// Build a vector of const char* for ImGui
  std::vector<const char *> optionPtrs;
  for (const auto &option : options) {
    optionPtrs.push_back(option.c_str());
  }

  int tempIndex = currentIndex;
  if (ImGui::Combo(name.c_str(), &tempIndex, optionPtrs.data(),
                   static_cast<int>(optionPtrs.size()))) {
    /// Selection changed, notify via callback
    if (onOptionChange_ && tempIndex >= 0 &&
        tempIndex < static_cast<int>(options.size())) {
      onOptionChange_(name, options[tempIndex]);
    }
  }
}

ImVec2 ToolOptionsPanel::getDefaultSize() {
  /// Return default panel size in pixels
  return ImVec2(250.0f, 400.0f);
}

} // namespace ImGUI
} // namespace view
