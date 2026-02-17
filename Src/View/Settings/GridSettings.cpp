/**
 * @file GridSettings.cpp
 * @brief Implementation of the Grid Settings dialog
 */

#include "GridSettings.hpp"

#include <spdlog/spdlog.h>

namespace view {
namespace settings {

GridSettings::GridSettings()
    : isOpen_(false), workingConfig_(), savedConfig_() {}

bool GridSettings::Render(GridConfig &config) {
  bool modified = false;

  if (!isOpen_) {
    return false;
  }

  // Set dialog size
  ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

  // Begin the dialog
  if (ImGui::Begin("Grid Settings", &isOpen_, ImGuiWindowFlags_NoCollapse)) {
    // Store the working config at the start
    if (workingConfig_ != config) {
      workingConfig_ = config;
      savedConfig_ = config;
    }

    // Render sections
    if (RenderVisibilitySection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderSpacingSection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderColorSection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderTypeSection()) {
      modified = true;
    }
    ImGui::Separator();

    // Render buttons
    if (RenderButtons(config)) {
      isOpen_ = false;
    }
  } else {
    isOpen_ = false;
  }
  ImGui::End();

  return modified;
}

void GridSettings::Open() {
  isOpen_ = true;
  spdlog::info("Grid Settings dialog opened");
}

void GridSettings::Close() {
  isOpen_ = false;
  spdlog::info("Grid Settings dialog closed");
}

bool GridSettings::IsOpen() const { return isOpen_; }

bool GridSettings::HasChanged() const { return workingConfig_ != savedConfig_; }

void GridSettings::SetOnApply(
    std::function<void(const GridConfig &)> callback) {
  onApply_ = std::move(callback);
}

void GridSettings::ResetToDefaults(GridConfig &config) {
  config = GridConfig();
  spdlog::info("Grid settings reset to defaults");
}

bool GridSettings::RenderVisibilitySection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Visibility", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Show grid toggle
    if (ImGui::Checkbox("Show Grid", &workingConfig_.showGrid)) {
      modified = true;
      spdlog::debug("Grid visibility changed to: {}", workingConfig_.showGrid);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Toggle grid visibility on the canvas");
    }

    // Show minor grid lines toggle
    if (ImGui::Checkbox("Show Minor Grid Lines",
                        &workingConfig_.showMinorGridLines)) {
      modified = true;
      spdlog::debug("Minor grid lines visibility changed to: {}",
                    workingConfig_.showMinorGridLines);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "Display minor grid lines between major grid lines for finer "
          "precision");
    }
  }

  return modified;
}

bool GridSettings::RenderSpacingSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Spacing", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Major grid spacing
    if (ImGui::SliderFloat("Major Grid Spacing",
                           &workingConfig_.majorGridSpacing, 1.0f, 100.0f,
                           "%.1f")) {
      modified = true;
      spdlog::debug("Major grid spacing changed to: {}",
                    workingConfig_.majorGridSpacing);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "Distance between major grid lines in world units (1-100)");
    }

    // Minor divisions
    if (ImGui::SliderInt("Minor Divisions", &workingConfig_.minorDivisions, 1,
                         20, "%d")) {
      modified = true;
      spdlog::debug("Minor divisions changed to: {}",
                    workingConfig_.minorDivisions);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "Number of minor divisions between each major grid line (1-20)");
    }

    // Grid opacity
    if (ImGui::SliderFloat("Grid Opacity", &workingConfig_.gridOpacity, 0.0f,
                           1.0f, "%.2f")) {
      modified = true;
      spdlog::debug("Grid opacity changed to: {}", workingConfig_.gridOpacity);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
          "Opacity of the grid (0.0 = transparent, 1.0 = opaque)");
    }
  }

  return modified;
}

bool GridSettings::RenderColorSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Major grid color
    if (ImGui::ColorEdit4("Major Grid Color##grid",
                          &workingConfig_.majorGridColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Major grid color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Color of major grid lines");
    }

    // Minor grid color
    if (ImGui::ColorEdit4("Minor Grid Color##grid",
                          &workingConfig_.minorGridColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Minor grid color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Color of minor grid lines");
    }
  }

  return modified;
}

bool GridSettings::RenderTypeSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Grid Type", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Grid type selection
    int gridType = static_cast<int>(workingConfig_.gridType);

    if (ImGui::RadioButton("Cartesian", &gridType,
                           static_cast<int>(GridType::Cartesian))) {
      workingConfig_.gridType = GridType::Cartesian;
      modified = true;
      spdlog::debug("Grid type changed to Cartesian");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Standard X-Y grid with perpendicular lines");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Isometric", &gridType,
                           static_cast<int>(GridType::Isometric))) {
      workingConfig_.gridType = GridType::Isometric;
      modified = true;
      spdlog::debug("Grid type changed to Isometric");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("30-degree isometric grid for 3D sketching");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Polar", &gridType,
                           static_cast<int>(GridType::Polar))) {
      workingConfig_.gridType = GridType::Polar;
      modified = true;
      spdlog::debug("Grid type changed to Polar");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Radial grid with concentric circles and radial lines");
    }
  }

  return modified;
}

bool GridSettings::RenderButtons(GridConfig &config) {
  bool shouldClose = false;

  // Button layout
  float buttonWidth = 100.0f;
  float spacing = ImGui::GetStyle().ItemSpacing.x;
  float totalWidth = buttonWidth * 3 + spacing * 2;
  float startX = (ImGui::GetWindowWidth() - totalWidth) * 0.5f;

  ImGui::SetCursorPosX(startX);

  // Apply button
  if (ImGui::Button("Apply", ImVec2(buttonWidth, 0))) {
    config = workingConfig_;
    savedConfig_ = workingConfig_;
    if (onApply_) {
      onApply_(config);
    }
    spdlog::info("Grid settings applied");
    shouldClose = true;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Apply changes and close dialog");
  }
  ImGui::SameLine();

  // Cancel button
  if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
    workingConfig_ = savedConfig_;
    spdlog::info("Grid settings changes cancelled");
    shouldClose = true;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Discard changes and close dialog");
  }
  ImGui::SameLine();

  // Reset button
  if (ImGui::Button("Reset", ImVec2(buttonWidth, 0))) {
    ResetToDefaults(workingConfig_);
    spdlog::info("Grid settings reset to defaults");
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Reset all settings to default values");
  }

  return shouldClose;
}

} // namespace settings
} // namespace view
