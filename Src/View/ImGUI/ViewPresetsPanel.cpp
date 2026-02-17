#include <View/ImGUI/ViewPresetsPanel.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace view {

ViewPresetsPanel::ViewPresetsPanel(NavigationManager &navigationManager)
    : navigationManager_(navigationManager), isDirty_(true),
      lastUpdateTime_(std::chrono::steady_clock::now()) {}

void ViewPresetsPanel::render() {
  /// ALWAYS call Begin/End every frame to prevent window flickering
  /// Rate limiting is applied to content updates only, not rendering
  ImGui::Begin("View Presets");

  /// Rate limiting: only update content if at least 100ms has passed (max 10
  /// Hz)
  if (shouldUpdate()) {
    lastUpdateTime_ = std::chrono::steady_clock::now();
  }

  /// Get current view preset from NavigationManager (which queries
  /// UIFSMAdapter)
  ViewPreset currentPreset = navigationManager_.getCurrentViewPreset();

  /// Render panel content
  {
    /// 2D Presets Section
    if (ImGui::CollapsingHeader("2D Views", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Top2D)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Top2D, "Top 2D")) {
        navigationManager_.setViewPreset(ViewPreset::Top2D);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();
    }

    /// 3D Presets Section
    if (ImGui::CollapsingHeader("3D Views", ImGuiTreeNodeFlags_DefaultOpen)) {
      /// Top 3D
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Top3D)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Top3D, "Top")) {
        navigationManager_.setViewPreset(ViewPreset::Top3D);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Front
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Front)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Front, "Front")) {
        navigationManager_.setViewPreset(ViewPreset::Front);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Right
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Right)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Right, "Right")) {
        navigationManager_.setViewPreset(ViewPreset::Right);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Bottom
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Bottom)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Bottom, "Bottom")) {
        navigationManager_.setViewPreset(ViewPreset::Bottom);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Back
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Back)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Back, "Back")) {
        navigationManager_.setViewPreset(ViewPreset::Back);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Left
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Left)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Left, "Left")) {
        navigationManager_.setViewPreset(ViewPreset::Left);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();

      /// Isometric
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (currentPreset == ViewPreset::Isometric)
                                ? ImVec4(0.3f, 0.6f, 1.0f, 1.0f)
                                : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
      if (renderPresetButton(ViewPreset::Isometric, "Isometric")) {
        navigationManager_.setViewPreset(ViewPreset::Isometric);
        isDirty_ = true;
      }
      ImGui::PopStyleColor();
    }

    /// Navigation info
    ImGui::Separator();
    ImGui::Text("Current: %s", getPresetLabel(currentPreset));
  }
  ImGui::End();

  /// Clear dirty flag after UI has been rendered
  isDirty_ = false;
}

void ViewPresetsPanel::markDirty() { isDirty_ = true; }

bool ViewPresetsPanel::isDirty() const { return isDirty_; }

void ViewPresetsPanel::clearDirty() { isDirty_ = false; }

bool ViewPresetsPanel::renderPresetButton(ViewPreset preset,
                                          const char *label) {
  /// Render button with preset label
  if (ImGui::Button(label)) {
    spdlog::info("[ViewPresetsPanel::renderPresetButton] Button '{}' clicked, preset={}", 
                 label, static_cast<int>(preset));
    return true;
  }
  return false;
}

bool ViewPresetsPanel::shouldUpdate() const {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - lastUpdateTime_);
  return elapsed >= MIN_UPDATE_INTERVAL;
}

const char *ViewPresetsPanel::getPresetLabel(ViewPreset preset) {
  switch (preset) {
  case ViewPreset::Top2D:
    return "Top 2D";
  case ViewPreset::Top3D:
    return "Top";
  case ViewPreset::Front:
    return "Front";
  case ViewPreset::Right:
    return "Right";
  case ViewPreset::Bottom:
    return "Bottom";
  case ViewPreset::Back:
    return "Back";
  case ViewPreset::Left:
    return "Left";
  case ViewPreset::Isometric:
    return "Isometric";
  case ViewPreset::Custom:
    return "Custom";
  default:
    return "Unknown";
  }
}

} // namespace view
