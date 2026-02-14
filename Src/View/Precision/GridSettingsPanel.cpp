#include <View/Precision/GridSettingsPanel.hpp>

namespace view {

GridSettingsPanel::GridSettingsPanel(UIFSMAdapter *uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter) {}

void GridSettingsPanel::render() {
  // Query panel visibility from UIFSMAdapter (stateless coordinator pattern)
  bool visible = uiFSMAdapter_->getGridSettingsPanelVisible();
  if (!ImGui::Begin("Grid Settings", &visible)) {
    ImGui::End();
    // Update visibility state in UIFSMAdapter if changed by user
    if (visible != uiFSMAdapter_->getGridSettingsPanelVisible()) {
      uiFSMAdapter_->setGridSettingsPanelVisible(visible);
    }
    return;
  }

  renderDisplaySettings();
  ImGui::Separator();
  renderGridSettings();
  ImGui::Separator();
  renderAxisSettings();

  ImGui::End();

  // Update visibility state in UIFSMAdapter if changed by user
  if (visible != uiFSMAdapter_->getGridSettingsPanelVisible()) {
    uiFSMAdapter_->setGridSettingsPanelVisible(visible);
  }
}

void GridSettingsPanel::renderDisplaySettings() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  GridSettings settings = uiFSMAdapter_->getGridSettings();

  // Grid visibility checkbox
  bool visible = settings.visible;
  if (ImGui::Checkbox("Visible", &visible)) {
    settings.visible = visible;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Show minor lines checkbox
  bool showMinorLines = settings.showMinorLines;
  if (ImGui::Checkbox("Show Minor Lines", &showMinorLines)) {
    settings.showMinorLines = showMinorLines;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Major grid color picker
  glm::vec4 majorColor = settings.color;
  if (ImGui::ColorEdit3("Major Color", &majorColor[0])) {
    settings.color = majorColor;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Minor grid color picker
  glm::vec4 minorColor = settings.minorColor;
  if (ImGui::ColorEdit3("Minor Color", &minorColor[0])) {
    settings.minorColor = minorColor;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Grid opacity slider
  float opacity = settings.opacity;
  if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
    settings.opacity = opacity;
    uiFSMAdapter_->setGridSettings(settings);
  }
}

void GridSettingsPanel::renderGridSettings() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  GridSettings settings = uiFSMAdapter_->getGridSettings();

  // Major spacing drag float
  float majorSpacing = settings.majorSpacing;
  if (ImGui::DragFloat("Major Spacing", &majorSpacing, 0.5f, 1.0f, 100.0f,
                       "%.1f")) {
    settings.majorSpacing = majorSpacing;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Minor divisions slider
  int minorDivisions = settings.minorDivisions;
  if (ImGui::SliderInt("Minor Divisions", &minorDivisions, 1, 20)) {
    settings.minorDivisions = minorDivisions;
    uiFSMAdapter_->setGridSettings(settings);
  }
}

void GridSettingsPanel::renderAxisSettings() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  GridSettings settings = uiFSMAdapter_->getGridSettings();

  // Show axes checkbox
  bool showAxes = settings.showAxes;
  if (ImGui::Checkbox("Show Axes", &showAxes)) {
    settings.showAxes = showAxes;
    uiFSMAdapter_->setGridSettings(settings);
  }

  // Show origin checkbox
  bool showOrigin = settings.showOrigin;
  if (ImGui::Checkbox("Show Origin", &showOrigin)) {
    settings.showOrigin = showOrigin;
    uiFSMAdapter_->setGridSettings(settings);
  }
}

} // namespace view
