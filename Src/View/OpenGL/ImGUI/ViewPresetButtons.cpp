#include "View/OpenGL/ImGUI/ViewPresetButtons.hpp"
#include "View/Presets/ViewPresetManager.hpp"
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace view {

ViewPresetButtons::ViewPresetButtons(
    std::shared_ptr<ViewPresetManager> presetManager)
    : presetManager_(std::move(presetManager)) {
  spdlog::debug("ViewPresetButtons initialized");
}

void ViewPresetButtons::render() {
  if (!presetManager_) {
    return;
  }

  // Get current preset for highlighting
  ViewPreset currentPreset = presetManager_->getCurrentPreset();

  // Row 1: Top, Front, Right
  if (renderPresetButton(ViewPreset::Top3D, "Top")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Top3D);
    }
  }
  ImGui::SameLine(0.0f, BUTTON_SPACING);
  if (renderPresetButton(ViewPreset::Front, "Front")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Front);
    }
  }
  ImGui::SameLine(0.0f, BUTTON_SPACING);
  if (renderPresetButton(ViewPreset::Right, "Right")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Right);
    }
  }

  // Row 2: Bottom, Back, Left
  if (renderPresetButton(ViewPreset::Bottom, "Bottom")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Bottom);
    }
  }
  ImGui::SameLine(0.0f, BUTTON_SPACING);
  if (renderPresetButton(ViewPreset::Back, "Back")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Back);
    }
  }
  ImGui::SameLine(0.0f, BUTTON_SPACING);
  if (renderPresetButton(ViewPreset::Left, "Left")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Left);
    }
  }

  // Row 3: ISO (centered)
  // Calculate center offset: 3 buttons + 2 spacings = total width
  float totalWidth = 3.0f * BUTTON_WIDTH + 2.0f * BUTTON_SPACING;
  float isoOffset = (totalWidth - BUTTON_WIDTH) * 0.5f;
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + isoOffset);
  if (renderPresetButton(ViewPreset::Isometric, "ISO")) {
    if (onPresetClicked_) {
      onPresetClicked_(ViewPreset::Isometric);
    }
  }
}

void ViewPresetButtons::setOnPresetClicked(
    std::function<void(ViewPreset)> callback) {
  onPresetClicked_ = std::move(callback);
}

bool ViewPresetButtons::renderPresetButton(ViewPreset preset,
                                           const char *label) {
  bool isActive = isPresetActive(preset);

  // Apply active state styling
  if (isActive) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
  }

  // Render button
  bool clicked = ImGui::Button(label, ImVec2(BUTTON_WIDTH, BUTTON_HEIGHT));

  // Show tooltip on hover
  if (ImGui::IsItemHovered()) {
    std::string tooltip = ViewPresetManager::getPresetName(preset);
    std::string shortcut = ViewPresetManager::getPresetShortcut(preset);
    if (!shortcut.empty()) {
      tooltip += "\n[" + shortcut + "]";
    }
    ImGui::SetTooltip("%s", tooltip.c_str());
  }

  // Pop active state styling
  if (isActive) {
    ImGui::PopStyleColor(3);
  }

  return clicked;
}

bool ViewPresetButtons::isPresetActive(ViewPreset preset) const {
  if (!presetManager_) {
    return false;
  }
  return presetManager_->getCurrentPreset() == preset;
}

} // namespace view
