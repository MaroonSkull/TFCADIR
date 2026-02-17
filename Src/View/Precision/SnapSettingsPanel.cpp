#include <View/Precision/SnapSettingsPanel.hpp>

namespace view {

SnapSettingsPanel::SnapSettingsPanel(UIFSMAdapter *uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter) {}

void SnapSettingsPanel::render() {
  // Query panel visibility from UIFSMAdapter (stateless coordinator pattern)
  bool visible = uiFSMAdapter_->getSnapSettingsPanelVisible();
  if (!ImGui::Begin("Snap Settings", &visible)) {
    ImGui::End();
    // Update visibility state in UIFSMAdapter if changed by user
    if (visible != uiFSMAdapter_->getSnapSettingsPanelVisible()) {
      uiFSMAdapter_->setSnapSettingsPanelVisible(visible);
    }
    return;
  }

  renderSnapModes();
  ImGui::Separator();
  renderSnapSettings();

  ImGui::End();

  // Update visibility state in UIFSMAdapter if changed by user
  if (visible != uiFSMAdapter_->getSnapSettingsPanelVisible()) {
    uiFSMAdapter_->setSnapSettingsPanelVisible(visible);
  }
}

void SnapSettingsPanel::renderSnapModes() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  SnapSettings settings = uiFSMAdapter_->getSnapSettings();

  // Grid snap checkbox
  bool gridEnabled = settings.gridEnabled;
  if (ImGui::Checkbox("Grid Snap", &gridEnabled)) {
    settings.gridEnabled = gridEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Endpoint snap checkbox
  bool endpointEnabled = settings.endpointEnabled;
  if (ImGui::Checkbox("Endpoint Snap", &endpointEnabled)) {
    settings.endpointEnabled = endpointEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Midpoint snap checkbox
  bool midpointEnabled = settings.midpointEnabled;
  if (ImGui::Checkbox("Midpoint Snap", &midpointEnabled)) {
    settings.midpointEnabled = midpointEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Center snap checkbox
  bool centerEnabled = settings.centerEnabled;
  if (ImGui::Checkbox("Center Snap", &centerEnabled)) {
    settings.centerEnabled = centerEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Intersection snap checkbox
  bool intersectionEnabled = settings.intersectionEnabled;
  if (ImGui::Checkbox("Intersection Snap", &intersectionEnabled)) {
    settings.intersectionEnabled = intersectionEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Nearest point snap checkbox
  bool nearestEnabled = settings.nearestEnabled;
  if (ImGui::Checkbox("Nearest Point Snap", &nearestEnabled)) {
    settings.nearestEnabled = nearestEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Tangent snap checkbox
  bool tangentEnabled = settings.tangentEnabled;
  if (ImGui::Checkbox("Tangent Snap", &tangentEnabled)) {
    settings.tangentEnabled = tangentEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Perpendicular snap checkbox
  bool perpendicularEnabled = settings.perpendicularEnabled;
  if (ImGui::Checkbox("Perpendicular Snap", &perpendicularEnabled)) {
    settings.perpendicularEnabled = perpendicularEnabled;
    uiFSMAdapter_->setSnapSettings(settings);
  }
}

void SnapSettingsPanel::renderSnapSettings() {
  // Query current settings from UIFSMAdapter (stateless coordinator pattern)
  SnapSettings settings = uiFSMAdapter_->getSnapSettings();

  // Snap tolerance slider
  float tolerance = settings.tolerancePixels;
  if (ImGui::SliderFloat("Snap Tolerance (px)", &tolerance, 1.0f, 50.0f,
                         "%.1f")) {
    settings.tolerancePixels = tolerance;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Show indicators checkbox
  bool showIndicators = settings.showIndicators;
  if (ImGui::Checkbox("Show Indicators", &showIndicators)) {
    settings.showIndicators = showIndicators;
    uiFSMAdapter_->setSnapSettings(settings);
  }

  // Indicator color picker
  glm::vec4 indicatorColor = settings.indicatorColor;
  if (ImGui::ColorEdit3("Indicator Color", &indicatorColor[0])) {
    settings.indicatorColor = indicatorColor;
    uiFSMAdapter_->setSnapSettings(settings);
  }
}

} // namespace view
