#include "View/Presets/ViewPresetManager.hpp"
#include <View/Navigation/NavigationManager.hpp>
#include <spdlog/spdlog.h>

namespace view {

ViewPresetManager::ViewPresetManager(NavigationManager &navigationManager)
    : navigationManager_(navigationManager) {
  spdlog::debug("ViewPresetManager initialized");
}

void ViewPresetManager::applyPreset(ViewPreset preset, float duration) {
  spdlog::info("Applying view preset: {}", getPresetName(preset));
  navigationManager_.startTransition(preset, duration);
}

std::string ViewPresetManager::getPresetName(ViewPreset preset) {
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

std::string ViewPresetManager::getPresetShortcut(ViewPreset preset) {
  switch (preset) {
  case ViewPreset::Top3D:
    return "NumPad 7";
  case ViewPreset::Front:
    return "NumPad 1";
  case ViewPreset::Right:
    return "NumPad 3";
  case ViewPreset::Bottom:
    return "Ctrl+NumPad 7";
  case ViewPreset::Back:
    return "Ctrl+NumPad 1";
  case ViewPreset::Left:
    return "Ctrl+NumPad 3";
  case ViewPreset::Isometric:
    return "NumPad 5";
  case ViewPreset::Top2D:
    return ""; // No shortcut for 2D mode
  case ViewPreset::Custom:
    return ""; // No shortcut for custom views
  default:
    return "";
  }
}

ViewPreset ViewPresetManager::getCurrentPreset() const {
  return navigationManager_.getCurrentViewPreset();
}

glm::vec3 ViewPresetManager::getPresetCameraPosition(ViewPreset preset) {
  // Default camera distance from origin
  constexpr float distance = 10.0f;

  switch (preset) {
  case ViewPreset::Top2D:
  case ViewPreset::Top3D:
    // Looking down Y axis
    return glm::vec3(0.0f, distance, 0.0f);
  case ViewPreset::Front:
    // Looking toward +Z
    return glm::vec3(0.0f, 0.0f, distance);
  case ViewPreset::Right:
    // Looking toward +X
    return glm::vec3(distance, 0.0f, 0.0f);
  case ViewPreset::Bottom:
    // Looking up Y axis
    return glm::vec3(0.0f, -distance, 0.0f);
  case ViewPreset::Back:
    // Looking toward -Z
    return glm::vec3(0.0f, 0.0f, -distance);
  case ViewPreset::Left:
    // Looking toward -X
    return glm::vec3(-distance, 0.0f, 0.0f);
  case ViewPreset::Isometric:
    // Classic isometric view (equal angles to all three axes)
    return glm::vec3(distance, distance, distance);
  case ViewPreset::Custom:
  default:
    return glm::vec3(distance, distance, distance);
  }
}

glm::vec3 ViewPresetManager::getPresetCameraTarget(ViewPreset preset) {
  // All presets look at the origin
  (void)preset; // Suppress unused parameter warning
  return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 ViewPresetManager::getPresetCameraUp(ViewPreset preset) {
  switch (preset) {
  case ViewPreset::Top2D:
  case ViewPreset::Top3D:
    // When looking down Y, use -Z as up
    return glm::vec3(0.0f, 0.0f, -1.0f);
  case ViewPreset::Bottom:
    // When looking up Y, use +Z as up
    return glm::vec3(0.0f, 0.0f, 1.0f);
  case ViewPreset::Front:
  case ViewPreset::Back:
  case ViewPreset::Right:
  case ViewPreset::Left:
  case ViewPreset::Isometric:
  case ViewPreset::Custom:
  default:
    // Standard up vector for most views
    return glm::vec3(0.0f, 1.0f, 0.0f);
  }
}

} // namespace view
