#include "ThemeManager.hpp"
#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

// ==========================================================================
// Constructor / Destructor
// ==========================================================================

/**
 * @brief Construct a ThemeManager
 * @param fsmAdapter Reference to the UIFSMAdapter for state access
 */
ThemeManager::ThemeManager(UIFSMAdapter &fsmAdapter) : fsmAdapter_(fsmAdapter) {
  // Build the color name to member pointer map
  buildColorMap();

  // Initialize UIFSMAdapter with dark theme if not already set
  auto settings = fsmAdapter_.getThemeSettings();
  if (settings.preset == ThemePreset::Custom && !settings.useCustom) {
    settings.preset = ThemePreset::Dark;
    settings.customScheme = createDarkTheme();
    settings.useCustom = false;
    fsmAdapter_.setThemeSettings(settings);
  }

  spdlog::debug("ThemeManager initialized");
}

/**
 * @brief Destructor
 */
ThemeManager::~ThemeManager() = default;

// ==========================================================================
// Theme Management
// ==========================================================================

/**
 * @brief Set the current theme to a predefined preset
 */
void ThemeManager::setTheme(ThemePreset preset) {
  auto settings = fsmAdapter_.getThemeSettings();
  settings.preset = preset;
  settings.useCustom = false;

  // Update custom scheme to match the preset
  switch (preset) {
  case ThemePreset::Dark:
    settings.customScheme = createDarkTheme();
    break;
  case ThemePreset::Light:
    settings.customScheme = createLightTheme();
    break;
  case ThemePreset::HighContrast:
    settings.customScheme = createHighContrastTheme();
    break;
  case ThemePreset::Custom:
    // Keep current scheme for custom theme
    break;
  }

  fsmAdapter_.setThemeSettings(settings);
  spdlog::info("Theme changed to {}", presetToString(preset));
}

/**
 * @brief Set a custom theme with user-defined colors
 */
void ThemeManager::setCustomTheme(const ColorScheme &scheme) {
  auto settings = fsmAdapter_.getThemeSettings();
  settings.preset = ThemePreset::Custom;
  settings.customScheme = scheme;
  settings.useCustom = true;

  fsmAdapter_.setThemeSettings(settings);
  spdlog::info("Custom theme applied");
}

/**
 * @brief Get the current theme preset
 */
ThemePreset ThemeManager::getCurrentTheme() const {
  return fsmAdapter_.getThemeSettings().preset;
}

/**
 * @brief Get the current color scheme
 */
const ColorScheme &ThemeManager::getCurrentColorScheme() const {
  const auto &settings = fsmAdapter_.getThemeSettings();

  // Return custom scheme if useCustom is true, otherwise return preset scheme
  if (settings.useCustom) {
    return settings.customScheme;
  }

  // For non-custom themes, we need to return the preset scheme
  // Since we can't return a temporary, cache it in thread_local
  static thread_local ColorScheme cachedScheme;
  cachedScheme = getPresetScheme(settings.preset);
  return cachedScheme;
}

// ==========================================================================
// Color Access
// ==========================================================================

/**
 * @brief Get a color by name
 */
glm::vec4 ThemeManager::getColor(const std::string &colorName) const {
  auto it = colorMap_.find(colorName);
  if (it != colorMap_.end()) {
    const auto &settings = fsmAdapter_.getThemeSettings();
    const ColorScheme &scheme = settings.useCustom
                                    ? settings.customScheme
                                    : getPresetScheme(settings.preset);
    return scheme.*(it->second);
  }

  spdlog::warn("Unknown color name '{}', returning white", colorName);
  return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * @brief Set a color by name
 */
void ThemeManager::setColor(const std::string &colorName,
                            const glm::vec4 &color) {
  auto it = colorMap_.find(colorName);
  if (it != colorMap_.end()) {
    auto settings = fsmAdapter_.getThemeSettings();
    settings.customScheme.*(it->second) = color;
    settings.preset = ThemePreset::Custom;
    settings.useCustom = true;

    fsmAdapter_.setThemeSettings(settings);
    spdlog::debug("Color '{}' updated", colorName);
  } else {
    spdlog::warn("Unknown color name '{}', color not set", colorName);
  }
}

// ==========================================================================
// Persistence
// ==========================================================================

/**
 * @brief Save current theme to a YAML file
 */
bool ThemeManager::saveToFile(const std::string &filepath) {
  const auto &settings = fsmAdapter_.getThemeSettings();

  std::ofstream file(filepath);
  if (!file.is_open()) {
    spdlog::error("Failed to open theme file for writing: {}", filepath);
    return false;
  }

  file << "theme:\n";
  file << "  name: \""
       << (settings.preset == ThemePreset::Custom  ? "Custom"
           : settings.preset == ThemePreset::Dark  ? "Dark"
           : settings.preset == ThemePreset::Light ? "Light"
                                                   : "High Contrast")
       << "\"\n";
  file << "  preset: " << presetToString(settings.preset) << "\n";
  file << "  useCustom: " << (settings.useCustom ? "true" : "false") << "\n";
  file << "  colors:\n";

  // Write all colors from the color map
  for (const auto &[name, memberPtr] : colorMap_) {
    const glm::vec4 &color = settings.customScheme.*memberPtr;
    file << "    " << name << ": \"" << vec4ToHexString(color) << "\"\n";
  }

  file.close();
  spdlog::info("Theme saved to {}", filepath);
  return true;
}

/**
 * @brief Load theme from a YAML file
 */
bool ThemeManager::loadFromFile(const std::string &filepath) {
  // TODO: Implement full YAML parsing when yaml-cpp is integrated
  //
  // Required implementation:
  // 1. Parse YAML file with theme settings
  // 2. Create ThemeSettings from parsed data (preset, useCustom, customScheme)
  // 3. Call fsmAdapter_.setThemeSettings(settings)
  // 4. Return true on success, false on failure
  //
  // Reference: See SettingsDialog::loadFromFile for YAML parsing pattern
  spdlog::warn(
      "Theme loading from YAML not yet implemented - requires "
      "yaml-cpp integration (see SettingsDialog::loadFromFile for reference)");
  return false;
}

// ==========================================================================
// ImGUI Integration
// ==========================================================================

/**
 * @brief Apply the current theme to an ImGUI style
 */
void ThemeManager::applyTheme(ImGuiStyle &style) const {
  const auto &settings = fsmAdapter_.getThemeSettings();
  const ColorScheme &scheme = settings.useCustom
                                  ? settings.customScheme
                                  : getPresetScheme(settings.preset);

  // Apply ImGUI core colors
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(scheme.windowBg.r, scheme.windowBg.g, scheme.windowBg.b,
             scheme.windowBg.a);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(
      scheme.panelBg.r, scheme.panelBg.g, scheme.panelBg.b, scheme.panelBg.a);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(scheme.panelBg.r, scheme.panelBg.g,
                                          scheme.panelBg.b, scheme.panelBg.a);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(scheme.frameBg.r, scheme.frameBg.g,
                                          scheme.frameBg.b, scheme.frameBg.a);
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(scheme.frameBgHovered.r, scheme.frameBgHovered.g,
             scheme.frameBgHovered.b, scheme.frameBgHovered.a);
  style.Colors[ImGuiCol_FrameBgActive] =
      ImVec4(scheme.frameBgActive.r, scheme.frameBgActive.g,
             scheme.frameBgActive.b, scheme.frameBgActive.a);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(scheme.titleBg.r, scheme.titleBg.g,
                                          scheme.titleBg.b, scheme.titleBg.a);
  style.Colors[ImGuiCol_TitleBgActive] =
      ImVec4(scheme.titleBgActive.r, scheme.titleBgActive.g,
             scheme.titleBgActive.b, scheme.titleBgActive.a);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(
      scheme.titleBg.r, scheme.titleBg.g, scheme.titleBg.b, scheme.titleBg.a);
  style.Colors[ImGuiCol_MenuBarBg] =
      ImVec4(scheme.menuBarBg.r, scheme.menuBarBg.g, scheme.menuBarBg.b,
             scheme.menuBarBg.a);
  style.Colors[ImGuiCol_ScrollbarBg] =
      ImVec4(scheme.scrollbarBg.r, scheme.scrollbarBg.g, scheme.scrollbarBg.b,
             scheme.scrollbarBg.a);
  style.Colors[ImGuiCol_ScrollbarGrab] =
      ImVec4(scheme.scrollbarGrab.r, scheme.scrollbarGrab.g,
             scheme.scrollbarGrab.b, scheme.scrollbarGrab.a);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(scheme.scrollbarGrabHovered.r, scheme.scrollbarGrabHovered.g,
             scheme.scrollbarGrabHovered.b, scheme.scrollbarGrabHovered.a);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(scheme.scrollbarGrabActive.r, scheme.scrollbarGrabActive.g,
             scheme.scrollbarGrabActive.b, scheme.scrollbarGrabActive.a);
  style.Colors[ImGuiCol_CheckMark] =
      ImVec4(scheme.checkMark.r, scheme.checkMark.g, scheme.checkMark.b,
             scheme.checkMark.a);
  style.Colors[ImGuiCol_SliderGrab] =
      ImVec4(scheme.sliderGrab.r, scheme.sliderGrab.g, scheme.sliderGrab.b,
             scheme.sliderGrab.a);
  style.Colors[ImGuiCol_SliderGrabActive] =
      ImVec4(scheme.sliderGrabActive.r, scheme.sliderGrabActive.g,
             scheme.sliderGrabActive.b, scheme.sliderGrabActive.a);
  style.Colors[ImGuiCol_Button] = ImVec4(scheme.buttonBg.r, scheme.buttonBg.g,
                                         scheme.buttonBg.b, scheme.buttonBg.a);
  style.Colors[ImGuiCol_ButtonHovered] =
      ImVec4(scheme.buttonBgHovered.r, scheme.buttonBgHovered.g,
             scheme.buttonBgHovered.b, scheme.buttonBgHovered.a);
  style.Colors[ImGuiCol_ButtonActive] =
      ImVec4(scheme.buttonBgActive.r, scheme.buttonBgActive.g,
             scheme.buttonBgActive.b, scheme.buttonBgActive.a);
  style.Colors[ImGuiCol_Header] = ImVec4(scheme.headerBg.r, scheme.headerBg.g,
                                         scheme.headerBg.b, scheme.headerBg.a);
  style.Colors[ImGuiCol_HeaderHovered] =
      ImVec4(scheme.headerBgHovered.r, scheme.headerBgHovered.g,
             scheme.headerBgHovered.b, scheme.headerBgHovered.a);
  style.Colors[ImGuiCol_HeaderActive] =
      ImVec4(scheme.headerBgActive.r, scheme.headerBgActive.g,
             scheme.headerBgActive.b, scheme.headerBgActive.a);
  style.Colors[ImGuiCol_Separator] = ImVec4(scheme.border.r, scheme.border.g,
                                            scheme.border.b, scheme.border.a);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(
      scheme.border.r, scheme.border.g, scheme.border.b, scheme.border.a);
  style.Colors[ImGuiCol_SeparatorActive] =
      ImVec4(scheme.borderActive.r, scheme.borderActive.g,
             scheme.borderActive.b, scheme.borderActive.a);
  style.Colors[ImGuiCol_ResizeGrip] =
      ImVec4(scheme.border.r, scheme.border.g, scheme.border.b,
             scheme.border.a * 0.5f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(
      scheme.accent.r, scheme.accent.g, scheme.accent.b, scheme.accent.a);
  style.Colors[ImGuiCol_ResizeGripActive] =
      ImVec4(scheme.accentHover.r, scheme.accentHover.g, scheme.accentHover.b,
             scheme.accentHover.a);
  style.Colors[ImGuiCol_Tab] = ImVec4(scheme.buttonBg.r, scheme.buttonBg.g,
                                      scheme.buttonBg.b, scheme.buttonBg.a);
  style.Colors[ImGuiCol_TabHovered] =
      ImVec4(scheme.buttonBgHovered.r, scheme.buttonBgHovered.g,
             scheme.buttonBgHovered.b, scheme.buttonBgHovered.a);
  style.Colors[ImGuiCol_TabActive] =
      ImVec4(scheme.buttonBgActive.r, scheme.buttonBgActive.g,
             scheme.buttonBgActive.b, scheme.buttonBgActive.a);
  style.Colors[ImGuiCol_TabUnfocused] =
      ImVec4(scheme.buttonBg.r, scheme.buttonBg.g, scheme.buttonBg.b,
             scheme.buttonBg.a * 0.7f);
  style.Colors[ImGuiCol_TabUnfocusedActive] =
      ImVec4(scheme.buttonBgActive.r, scheme.buttonBgActive.g,
             scheme.buttonBgActive.b, scheme.buttonBgActive.a * 0.7f);
  style.Colors[ImGuiCol_DockingPreview] =
      ImVec4(scheme.dragDropTarget.r, scheme.dragDropTarget.g,
             scheme.dragDropTarget.b, scheme.dragDropTarget.a);
  style.Colors[ImGuiCol_DockingEmptyBg] =
      ImVec4(scheme.panelBg.r, scheme.panelBg.g, scheme.panelBg.b,
             scheme.panelBg.a * 0.5f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(scheme.accent.r, scheme.accent.g,
                                            scheme.accent.b, scheme.accent.a);
  style.Colors[ImGuiCol_PlotLinesHovered] =
      ImVec4(scheme.accentHover.r, scheme.accentHover.g, scheme.accentHover.b,
             scheme.accentHover.a);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(
      scheme.accent.r, scheme.accent.g, scheme.accent.b, scheme.accent.a);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(scheme.accentHover.r, scheme.accentHover.g, scheme.accentHover.b,
             scheme.accentHover.a);
  style.Colors[ImGuiCol_TextSelectedBg] =
      ImVec4(scheme.selectionBg.r, scheme.selectionBg.g, scheme.selectionBg.b,
             scheme.selectionBg.a);
  style.Colors[ImGuiCol_DragDropTarget] =
      ImVec4(scheme.dragDropTarget.r, scheme.dragDropTarget.g,
             scheme.dragDropTarget.b, scheme.dragDropTarget.a);
  style.Colors[ImGuiCol_NavHighlight] =
      ImVec4(scheme.navHighlight.r, scheme.navHighlight.g,
             scheme.navHighlight.b, scheme.navHighlight.a);
  style.Colors[ImGuiCol_NavWindowingHighlight] =
      ImVec4(scheme.navWindowingHighlight.r, scheme.navWindowingHighlight.g,
             scheme.navWindowingHighlight.b, scheme.navWindowingHighlight.a);
  style.Colors[ImGuiCol_NavWindowingDimBg] =
      ImVec4(scheme.navWindowingDimBg.r, scheme.navWindowingDimBg.g,
             scheme.navWindowingDimBg.b, scheme.navWindowingDimBg.a);
  style.Colors[ImGuiCol_ModalWindowDimBg] =
      ImVec4(scheme.modalWindowDimBg.r, scheme.modalWindowDimBg.g,
             scheme.modalWindowDimBg.b, scheme.modalWindowDimBg.a);

  // Text colors
  style.Colors[ImGuiCol_Text] =
      ImVec4(scheme.text.r, scheme.text.g, scheme.text.b, scheme.text.a);
  style.Colors[ImGuiCol_TextDisabled] =
      ImVec4(scheme.textDisabled.r, scheme.textDisabled.g,
             scheme.textDisabled.b, scheme.textDisabled.a);
  style.Colors[ImGuiCol_TextSelectedBg] =
      ImVec4(scheme.selectionBg.r, scheme.selectionBg.g, scheme.selectionBg.b,
             scheme.selectionBg.a);
  style.Colors[ImGuiCol_TextLink] = ImVec4(scheme.accent.r, scheme.accent.g,
                                           scheme.accent.b, scheme.accent.a);

  // Border colors
  style.Colors[ImGuiCol_Border] = ImVec4(scheme.border.r, scheme.border.g,
                                         scheme.border.b, scheme.border.a);
  style.Colors[ImGuiCol_BorderShadow] =
      ImVec4(scheme.border.r, scheme.border.g, scheme.border.b,
             scheme.border.a * 0.5f);

  // Table colors
  style.Colors[ImGuiCol_TableHeaderBg] =
      ImVec4(scheme.tableHeaderBg.r, scheme.tableHeaderBg.g,
             scheme.tableHeaderBg.b, scheme.tableHeaderBg.a);
  style.Colors[ImGuiCol_TableBorderStrong] =
      ImVec4(scheme.tableBorderStrong.r, scheme.tableBorderStrong.g,
             scheme.tableBorderStrong.b, scheme.tableBorderStrong.a);
  style.Colors[ImGuiCol_TableBorderLight] =
      ImVec4(scheme.border.r, scheme.border.g, scheme.border.b,
             scheme.border.a * 0.5f);
  style.Colors[ImGuiCol_TableRowBg] = ImVec4(
      scheme.tableBg.r, scheme.tableBg.g, scheme.tableBg.b, scheme.tableBg.a);
  style.Colors[ImGuiCol_TableRowBgAlt] =
      ImVec4(scheme.tableRowBgAlt.r, scheme.tableRowBgAlt.g,
             scheme.tableRowBgAlt.b, scheme.tableRowBgAlt.a);
}

/**
 * @brief Render a theme editor UI
 */
void ThemeManager::renderThemeEditor() {
  // No mutex lock needed - ImGUI is single-threaded and UIFSMAdapter handles
  // its own synchronization

  auto settings = fsmAdapter_.getThemeSettings();
  ImGui::Text("Current Theme: %s", presetToString(settings.preset).c_str());

  // Theme preset buttons
  ImGui::Separator();
  ImGui::Text("Theme Presets");

  if (ImGui::Button("Dark Theme")) {
    setTheme(ThemePreset::Dark);
    settings = fsmAdapter_.getThemeSettings(); // Refresh after change
  }
  ImGui::SameLine();
  if (ImGui::Button("Light Theme")) {
    setTheme(ThemePreset::Light);
    settings = fsmAdapter_.getThemeSettings(); // Refresh after change
  }
  ImGui::SameLine();
  if (ImGui::Button("High Contrast")) {
    setTheme(ThemePreset::HighContrast);
    settings = fsmAdapter_.getThemeSettings(); // Refresh after change
  }

  // Color editor
  ImGui::Separator();

  if (ImGui::CollapsingHeader("Colors")) {
    // Get mutable reference to custom scheme for editing
    ColorScheme editableScheme = settings.customScheme;
    bool schemeChanged = false;

    for (auto &[name, memberPtr] : colorMap_) {
      glm::vec4 &color = editableScheme.*memberPtr;

      // Convert to float array for ImGui
      float col[4] = {color.r, color.g, color.b, color.a};

      if (ImGui::ColorEdit4(name.c_str(), col)) {
        color.r = col[0];
        color.g = col[1];
        color.b = col[2];
        color.a = col[3];
        schemeChanged = true;
      }
    }

    // Update settings if colors changed
    if (schemeChanged) {
      settings.customScheme = editableScheme;
      settings.preset = ThemePreset::Custom;
      settings.useCustom = true;
      fsmAdapter_.setThemeSettings(settings);
    }
  }
}

// ==========================================================================
// Preset Access
// ==========================================================================

/**
 * @brief Get a predefined color scheme
 */
ColorScheme ThemeManager::getPresetScheme(ThemePreset preset) const {
  switch (preset) {
  case ThemePreset::Dark:
    return createDarkTheme();
  case ThemePreset::Light:
    return createLightTheme();
  case ThemePreset::HighContrast:
    return createHighContrastTheme();
  case ThemePreset::Custom:
    // Return the custom scheme from UIFSMAdapter
    return fsmAdapter_.getThemeSettings().customScheme;
  default:
    return createDarkTheme();
  }
}

// ==========================================================================
// Private Methods
// ==========================================================================

/**
 * @brief Initialize the color name to member pointer map
 */
void ThemeManager::buildColorMap() {
  // ImGUI Core Colors
  colorMap_["windowBg"] = &ColorScheme::windowBg;
  colorMap_["panelBg"] = &ColorScheme::panelBg;
  colorMap_["text"] = &ColorScheme::text;
  colorMap_["textDisabled"] = &ColorScheme::textDisabled;
  colorMap_["accent"] = &ColorScheme::accent;
  colorMap_["accentHover"] = &ColorScheme::accentHover;
  colorMap_["border"] = &ColorScheme::border;
  colorMap_["borderActive"] = &ColorScheme::borderActive;
  colorMap_["buttonBg"] = &ColorScheme::buttonBg;
  colorMap_["buttonBgHovered"] = &ColorScheme::buttonBgHovered;
  colorMap_["buttonBgActive"] = &ColorScheme::buttonBgActive;
  colorMap_["headerBg"] = &ColorScheme::headerBg;
  colorMap_["headerBgHovered"] = &ColorScheme::headerBgHovered;
  colorMap_["headerBgActive"] = &ColorScheme::headerBgActive;
  colorMap_["frameBg"] = &ColorScheme::frameBg;
  colorMap_["frameBgHovered"] = &ColorScheme::frameBgHovered;
  colorMap_["frameBgActive"] = &ColorScheme::frameBgActive;
  colorMap_["titleBg"] = &ColorScheme::titleBg;
  colorMap_["titleBgActive"] = &ColorScheme::titleBgActive;
  colorMap_["titleText"] = &ColorScheme::titleText;
  colorMap_["titleTextActive"] = &ColorScheme::titleTextActive;
  colorMap_["menuBarBg"] = &ColorScheme::menuBarBg;
  colorMap_["scrollbarBg"] = &ColorScheme::scrollbarBg;
  colorMap_["scrollbarGrab"] = &ColorScheme::scrollbarGrab;
  colorMap_["scrollbarGrabHovered"] = &ColorScheme::scrollbarGrabHovered;
  colorMap_["scrollbarGrabActive"] = &ColorScheme::scrollbarGrabActive;
  colorMap_["checkMark"] = &ColorScheme::checkMark;
  colorMap_["sliderGrab"] = &ColorScheme::sliderGrab;
  colorMap_["sliderGrabActive"] = &ColorScheme::sliderGrabActive;
  colorMap_["tableBg"] = &ColorScheme::tableBg;
  colorMap_["tableHeaderBg"] = &ColorScheme::tableHeaderBg;
  colorMap_["tableRowBgAlt"] = &ColorScheme::tableRowBgAlt;
  colorMap_["tableBorderStrong"] = &ColorScheme::tableBorderStrong;
  colorMap_["selectionBg"] = &ColorScheme::selectionBg;
  colorMap_["selectionText"] = &ColorScheme::selectionText;
  colorMap_["dragDropTarget"] = &ColorScheme::dragDropTarget;
  colorMap_["navHighlight"] = &ColorScheme::navHighlight;
  colorMap_["navWindowingHighlight"] = &ColorScheme::navWindowingHighlight;
  colorMap_["navWindowingDimBg"] = &ColorScheme::navWindowingDimBg;
  colorMap_["modalWindowDimBg"] = &ColorScheme::modalWindowDimBg;

  // CAD-Specific Colors
  colorMap_["gridMajor"] = &ColorScheme::gridMajor;
  colorMap_["gridMinor"] = &ColorScheme::gridMinor;
  colorMap_["axisX"] = &ColorScheme::axisX;
  colorMap_["axisY"] = &ColorScheme::axisY;
  colorMap_["axisZ"] = &ColorScheme::axisZ;
  colorMap_["selection"] = &ColorScheme::selection;
  colorMap_["snap"] = &ColorScheme::snap;
  colorMap_["preview"] = &ColorScheme::preview;
  colorMap_["cursor"] = &ColorScheme::cursor;
  colorMap_["highlight"] = &ColorScheme::highlight;
  colorMap_["error"] = &ColorScheme::error;
  colorMap_["success"] = &ColorScheme::success;
  colorMap_["info"] = &ColorScheme::info;
}

/**
 * @brief Create the dark theme color scheme
 */
ColorScheme ThemeManager::createDarkTheme() const {
  ColorScheme scheme;

  // ImGUI Core Colors - Professional Dark CAD Theme
  scheme.windowBg = hexToVec4(0x1E1E1E);     // #1E1E1E (30,30,30)
  scheme.panelBg = hexToVec4(0x252526);      // #252526 (37,37,38)
  scheme.text = hexToVec4(0xCCCCCC);         // #CCCCCC (204,204,204)
  scheme.textDisabled = hexToVec4(0x888888); // #888888 (136,136,136)
  scheme.accent = hexToVec4(0x007ACC);       // #007ACC (0,122,204)
  scheme.accentHover = hexToVec4(0x1E90FF);  // #1E90FF (30,144,255)
  scheme.border = hexToVec4(0x3E3E42);       // #3E3E42 (62,62,66)
  scheme.borderActive = hexToVec4(0x007ACC); // #007ACC (0,122,204)

  // Button colors
  scheme.buttonBg = hexToVec4(0x3C3C3C);        // #3C3C3C (60,60,60)
  scheme.buttonBgHovered = hexToVec4(0x4A4A4A); // #4A4A4A (74,74,74)
  scheme.buttonBgActive = hexToVec4(0x007ACC);  // #007ACC (0,122,204)

  // Header colors
  scheme.headerBg = hexToVec4(0x2D2D30);        // #2D2D30 (45,45,48)
  scheme.headerBgHovered = hexToVec4(0x3E3E42); // #3E3E42 (62,62,66)
  scheme.headerBgActive = hexToVec4(0x007ACC);  // #007ACC (0,122,204)

  // Frame/input colors
  scheme.frameBg = hexToVec4(0x2D2D30);        // #2D2D30 (45,45,48)
  scheme.frameBgHovered = hexToVec4(0x3E3E42); // #3E3E42 (62,62,66)
  scheme.frameBgActive = hexToVec4(0x007ACC);  // #007ACC (0,122,204)

  // Title bar colors
  scheme.titleBg = hexToVec4(0x2D2D30);         // #2D2D30 (45,45,48)
  scheme.titleBgActive = hexToVec4(0x007ACC);   // #007ACC (0,122,204)
  scheme.titleText = hexToVec4(0xCCCCCC);       // #CCCCCC (204,204,204)
  scheme.titleTextActive = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)

  // Menu bar
  scheme.menuBarBg = hexToVec4(0x2D2D30); // #2D2D30 (45,45,48)

  // Scrollbar
  scheme.scrollbarBg = hexToVec4(0x2D2D30);          // #2D2D30 (45,45,48)
  scheme.scrollbarGrab = hexToVec4(0x555555);        // #555555 (85,85,85)
  scheme.scrollbarGrabHovered = hexToVec4(0x666666); // #666666 (102,102,102)
  scheme.scrollbarGrabActive = hexToVec4(0x007ACC);  // #007ACC (0,122,204)

  // Interactive elements
  scheme.checkMark = hexToVec4(0x007ACC);        // #007ACC (0,122,204)
  scheme.sliderGrab = hexToVec4(0x007ACC);       // #007ACC (0,122,204)
  scheme.sliderGrabActive = hexToVec4(0x1E90FF); // #1E90FF (30,144,255)

  // Table colors
  scheme.tableBg = hexToVec4(0x1E1E1E);           // #1E1E1E (30,30,30)
  scheme.tableHeaderBg = hexToVec4(0x2D2D30);     // #2D2D30 (45,45,48)
  scheme.tableRowBgAlt = hexToVec4(0x252526);     // #252526 (37,37,38)
  scheme.tableBorderStrong = hexToVec4(0x3E3E42); // #3E3E42 (62,62,66)

  // Selection
  scheme.selectionBg = hexToVec4(0x007ACC);   // #007ACC (0,122,204)
  scheme.selectionText = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)

  // Drag and drop
  scheme.dragDropTarget = hexToVec4(0x007ACC); // #007ACC (0,122,204)

  // Navigation
  scheme.navHighlight = hexToVec4(0x007ACC);          // #007ACC (0,122,204)
  scheme.navWindowingHighlight = hexToVec4(0x1E90FF); // #1E90FF (30,144,255)
  scheme.navWindowingDimBg = hexToVec4(0x00000080);   // #00000080 (0,0,0,128)
  scheme.modalWindowDimBg = hexToVec4(0x00000080);    // #00000080 (0,0,0,128)

  // CAD-Specific Colors
  scheme.gridMajor = hexToVec4(0x444444); // #444444 (68,68,68)
  scheme.gridMinor = hexToVec4(0x333333); // #333333 (51,51,51)
  scheme.axisX = hexToVec4(0xFF6B6B);     // #FF6B6B (255,107,107)
  scheme.axisY = hexToVec4(0x4ECDC4);     // #4ECDC4 (78,205,196)
  scheme.axisZ = hexToVec4(0xFFE66D);     // #FFE66D (255,230,109)
  scheme.selection = hexToVec4(0xFFA500); // #FFA500 (255,165,0)
  scheme.snap = hexToVec4(0x00FF00);      // #00FF00 (0,255,0)
  scheme.preview = hexToVec4(0xFFFFFF80); // #FFFFFF80 (255,255,255,128)
  scheme.cursor = hexToVec4(0xFFFFFF);    // #FFFFFF (255,255,255)
  scheme.highlight = hexToVec4(0x007ACC); // #007ACC (0,122,204)
  scheme.error = hexToVec4(0xF44336);     // #F44336 (244,67,54)
  scheme.success = hexToVec4(0x4CAF50);   // #4CAF50 (76,175,80)
  scheme.info = hexToVec4(0x2196F3);      // #2196F3 (33,150,243)

  return scheme;
}

/**
 * @brief Create the light theme color scheme
 */
ColorScheme ThemeManager::createLightTheme() const {
  ColorScheme scheme;

  // ImGUI Core Colors - Light Theme
  scheme.windowBg = hexToVec4(0xFFFFFF);     // #FFFFFF (255,255,255)
  scheme.panelBg = hexToVec4(0xF3F3F3);      // #F3F3F3 (243,243,243)
  scheme.text = hexToVec4(0x000000);         // #000000 (0,0,0)
  scheme.textDisabled = hexToVec4(0x888888); // #888888 (136,136,136)
  scheme.accent = hexToVec4(0x0066CC);       // #0066CC (0,102,204)
  scheme.accentHover = hexToVec4(0x0052A3);  // #0052A3 (0,82,163)
  scheme.border = hexToVec4(0xCCCCCC);       // #CCCCCC (204,204,204)
  scheme.borderActive = hexToVec4(0x0066CC); // #0066CC (0,102,204)

  // Button colors
  scheme.buttonBg = hexToVec4(0xE0E0E0);        // #E0E0E0 (224,224,224)
  scheme.buttonBgHovered = hexToVec4(0xD0D0D0); // #D0D0D0 (208,208,208)
  scheme.buttonBgActive = hexToVec4(0x0066CC);  // #0066CC (0,102,204)

  // Header colors
  scheme.headerBg = hexToVec4(0xE8E8E8);        // #E8E8E8 (232,232,232)
  scheme.headerBgHovered = hexToVec4(0xD8D8D8); // #D8D8D8 (216,216,216)
  scheme.headerBgActive = hexToVec4(0x0066CC);  // #0066CC (0,102,204)

  // Frame/input colors
  scheme.frameBg = hexToVec4(0xFFFFFF);        // #FFFFFF (255,255,255)
  scheme.frameBgHovered = hexToVec4(0xF0F0F0); // #F0F0F0 (240,240,240)
  scheme.frameBgActive = hexToVec4(0x0066CC);  // #0066CC (0,102,204)

  // Title bar colors
  scheme.titleBg = hexToVec4(0xE8E8E8);         // #E8E8E8 (232,232,232)
  scheme.titleBgActive = hexToVec4(0x0066CC);   // #0066CC (0,102,204)
  scheme.titleText = hexToVec4(0x000000);       // #000000 (0,0,0)
  scheme.titleTextActive = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)

  // Menu bar
  scheme.menuBarBg = hexToVec4(0xF0F0F0); // #F0F0F0 (240,240,240)

  // Scrollbar
  scheme.scrollbarBg = hexToVec4(0xF0F0F0);          // #F0F0F0 (240,240,240)
  scheme.scrollbarGrab = hexToVec4(0xCCCCCC);        // #CCCCCC (204,204,204)
  scheme.scrollbarGrabHovered = hexToVec4(0xBBBBBB); // #BBBBBB (187,187,187)
  scheme.scrollbarGrabActive = hexToVec4(0x0066CC);  // #0066CC (0,102,204)

  // Interactive elements
  scheme.checkMark = hexToVec4(0x0066CC);        // #0066CC (0,102,204)
  scheme.sliderGrab = hexToVec4(0x0066CC);       // #0066CC (0,102,204)
  scheme.sliderGrabActive = hexToVec4(0x0052A3); // #0052A3 (0,82,163)

  // Table colors
  scheme.tableBg = hexToVec4(0xFFFFFF);           // #FFFFFF (255,255,255)
  scheme.tableHeaderBg = hexToVec4(0xF0F0F0);     // #F0F0F0 (240,240,240)
  scheme.tableRowBgAlt = hexToVec4(0xF9F9F9);     // #F9F9F9 (249,249,249)
  scheme.tableBorderStrong = hexToVec4(0xCCCCCC); // #CCCCCC (204,204,204)

  // Selection
  scheme.selectionBg = hexToVec4(0x0066CC);   // #0066CC (0,102,204)
  scheme.selectionText = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)

  // Drag and drop
  scheme.dragDropTarget = hexToVec4(0x0066CC); // #0066CC (0,102,204)

  // Navigation
  scheme.navHighlight = hexToVec4(0x0066CC);          // #0066CC (0,102,204)
  scheme.navWindowingHighlight = hexToVec4(0x0052A3); // #0052A3 (0,82,163)
  scheme.navWindowingDimBg = hexToVec4(0x00000040);   // #00000040 (0,0,0,64)
  scheme.modalWindowDimBg = hexToVec4(0x00000040);    // #00000040 (0,0,0,64)

  // CAD-Specific Colors
  scheme.gridMajor = hexToVec4(0xCCCCCC); // #CCCCCC (204,204,204)
  scheme.gridMinor = hexToVec4(0xE5E5E5); // #E5E5E5 (229,229,229)
  scheme.axisX = hexToVec4(0xFF6B6B);     // #FF6B6B (255,107,107)
  scheme.axisY = hexToVec4(0x4ECDC4);     // #4ECDC4 (78,205,196)
  scheme.axisZ = hexToVec4(0xFFE66D);     // #FFE66D (255,230,109)
  scheme.selection = hexToVec4(0xFFA500); // #FFA500 (255,165,0)
  scheme.snap = hexToVec4(0x00CC00);      // #00CC00 (0,204,0)
  scheme.preview = hexToVec4(0x00000080); // #00000080 (0,0,0,128)
  scheme.cursor = hexToVec4(0x000000);    // #000000 (0,0,0)
  scheme.highlight = hexToVec4(0x0066CC); // #0066CC (0,102,204)
  scheme.error = hexToVec4(0xD32F2F);     // #D32F2F (211,47,47)
  scheme.success = hexToVec4(0x388E3C);   // #388E3C (56,142,60)
  scheme.info = hexToVec4(0x1976D2);      // #1976D2 (25,118,210)

  return scheme;
}

/**
 * @brief Create the high contrast theme color scheme
 */
ColorScheme ThemeManager::createHighContrastTheme() const {
  ColorScheme scheme;

  // ImGUI Core Colors - High Contrast Theme
  scheme.windowBg = hexToVec4(0x000000);     // #000000 (0,0,0)
  scheme.panelBg = hexToVec4(0x1A1A1A);      // #1A1A1A (26,26,26)
  scheme.text = hexToVec4(0xFFFFFF);         // #FFFFFF (255,255,255)
  scheme.textDisabled = hexToVec4(0xAAAAAA); // #AAAAAA (170,170,170)
  scheme.accent = hexToVec4(0xFFFF00);       // #FFFF00 (255,255,0)
  scheme.accentHover = hexToVec4(0xFFEE00);  // #FFEE00 (255,238,0)
  scheme.border = hexToVec4(0xFFFFFF);       // #FFFFFF (255,255,255)
  scheme.borderActive = hexToVec4(0xFFFF00); // #FFFF00 (255,255,0)

  // Button colors
  scheme.buttonBg = hexToVec4(0x333333);        // #333333 (51,51,51)
  scheme.buttonBgHovered = hexToVec4(0x444444); // #444444 (68,68,68)
  scheme.buttonBgActive = hexToVec4(0xFFFF00);  // #FFFF00 (255,255,0)

  // Header colors
  scheme.headerBg = hexToVec4(0x333333);        // #333333 (51,51,51)
  scheme.headerBgHovered = hexToVec4(0x444444); // #444444 (68,68,68)
  scheme.headerBgActive = hexToVec4(0xFFFF00);  // #FFFF00 (255,255,0)

  // Frame/input colors
  scheme.frameBg = hexToVec4(0x000000);        // #000000 (0,0,0)
  scheme.frameBgHovered = hexToVec4(0x222222); // #222222 (34,34,34)
  scheme.frameBgActive = hexToVec4(0xFFFF00);  // #FFFF00 (255,255,0)

  // Title bar colors
  scheme.titleBg = hexToVec4(0x333333);         // #333333 (51,51,51)
  scheme.titleBgActive = hexToVec4(0xFFFF00);   // #FFFF00 (255,255,0)
  scheme.titleText = hexToVec4(0xFFFFFF);       // #FFFFFF (255,255,255)
  scheme.titleTextActive = hexToVec4(0x000000); // #000000 (0,0,0)

  // Menu bar
  scheme.menuBarBg = hexToVec4(0x333333); // #333333 (51,51,51)

  // Scrollbar
  scheme.scrollbarBg = hexToVec4(0x333333);          // #333333 (51,51,51)
  scheme.scrollbarGrab = hexToVec4(0xFFFFFF);        // #FFFFFF (255,255,255)
  scheme.scrollbarGrabHovered = hexToVec4(0xEEEEEE); // #EEEEEE (238,238,238)
  scheme.scrollbarGrabActive = hexToVec4(0xFFFF00);  // #FFFF00 (255,255,0)

  // Interactive elements
  scheme.checkMark = hexToVec4(0xFFFF00);        // #FFFF00 (255,255,0)
  scheme.sliderGrab = hexToVec4(0xFFFF00);       // #FFFF00 (255,255,0)
  scheme.sliderGrabActive = hexToVec4(0xFFEE00); // #FFEE00 (255,238,0)

  // Table colors
  scheme.tableBg = hexToVec4(0x000000);           // #000000 (0,0,0)
  scheme.tableHeaderBg = hexToVec4(0x333333);     // #333333 (51,51,51)
  scheme.tableRowBgAlt = hexToVec4(0x1A1A1A);     // #1A1A1A (26,26,26)
  scheme.tableBorderStrong = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)

  // Selection
  scheme.selectionBg = hexToVec4(0xFFFF00);   // #FFFF00 (255,255,0)
  scheme.selectionText = hexToVec4(0x000000); // #000000 (0,0,0)

  // Drag and drop
  scheme.dragDropTarget = hexToVec4(0xFFFF00); // #FFFF00 (255,255,0)

  // Navigation
  scheme.navHighlight = hexToVec4(0xFFFF00);          // #FFFF00 (255,255,0)
  scheme.navWindowingHighlight = hexToVec4(0xFFEE00); // #FFEE00 (255,238,0)
  scheme.navWindowingDimBg = hexToVec4(0x000000C0);   // #000000C0 (0,0,0,192)
  scheme.modalWindowDimBg = hexToVec4(0x000000C0);    // #000000C0 (0,0,0,192)

  // CAD-Specific Colors
  scheme.gridMajor = hexToVec4(0xFFFFFF); // #FFFFFF (255,255,255)
  scheme.gridMinor = hexToVec4(0x888888); // #888888 (136,136,136)
  scheme.axisX = hexToVec4(0xFF0000);     // #FF0000 (255,0,0)
  scheme.axisY = hexToVec4(0x00FF00);     // #00FF00 (0,255,0)
  scheme.axisZ = hexToVec4(0x0000FF);     // #0000FF (0,0,255)
  scheme.selection = hexToVec4(0xFFFF00); // #FFFF00 (255,255,0)
  scheme.snap = hexToVec4(0x00FFFF);      // #00FFFF (0,255,255)
  scheme.preview = hexToVec4(0xFFFFFFFF); // #FFFFFFFF (255,255,255,255)
  scheme.cursor = hexToVec4(0xFFFFFF);    // #FFFFFF (255,255,255)
  scheme.highlight = hexToVec4(0xFFFF00); // #FFFF00 (255,255,0)
  scheme.error = hexToVec4(0xFF0000);     // #FF0000 (255,0,0)
  scheme.success = hexToVec4(0x00FF00);   // #00FF00 (0,255,0)
  scheme.info = hexToVec4(0x00FFFF);      // #00FFFF (0,255,255)

  return scheme;
}

/**
 * @brief Convert hex color to glm::vec4
 */
glm::vec4 ThemeManager::hexToVec4(uint32_t hex) const {
  glm::vec4 result;
  result.r = ((hex >> 16) & 0xFF) / 255.0f;
  result.g = ((hex >> 8) & 0xFF) / 255.0f;
  result.b = (hex & 0xFF) / 255.0f;
  result.a = ((hex >> 24) & 0xFF) / 255.0f;
  return result;
}

/**
 * @brief Convert glm::vec4 to hex color
 */
uint32_t ThemeManager::vec4ToHex(const glm::vec4 &color) const {
  uint32_t result = 0;
  result |= static_cast<uint32_t>(color.a * 255.0f) << 24;
  result |= static_cast<uint32_t>(color.r * 255.0f) << 16;
  result |= static_cast<uint32_t>(color.g * 255.0f) << 8;
  result |= static_cast<uint32_t>(color.b * 255.0f);
  return result;
}

/**
 * @brief Convert hex string to glm::vec4
 */
glm::vec4 ThemeManager::hexStringToVec4(const std::string &hexString) const {
  try {
    std::string str = hexString;

    // Remove leading '#' if present
    if (!str.empty() && str[0] == '#') {
      str = str.substr(1);
    }

    // Parse hex value
    uint32_t hex = std::stoul(str, nullptr, 16);

    // Handle RGB format (no alpha)
    if (str.length() == 6) {
      hex |= 0xFF000000; // Set alpha to 255
    }

    return hexToVec4(hex);
  } catch (const std::exception &e) {
    spdlog::error("Invalid hex color string '{}': {}", hexString, e.what());
    return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Return white as default
  }
}

/**
 * @brief Convert glm::vec4 to hex string
 */
std::string ThemeManager::vec4ToHexString(const glm::vec4 &color) const {
  uint32_t hex = vec4ToHex(color);
  char buffer[10];
  snprintf(
      buffer, sizeof(buffer), "#%02X%02X%02X%02X",
      static_cast<int>(color.a * 255.0f), static_cast<int>(color.r * 255.0f),
      static_cast<int>(color.g * 255.0f), static_cast<int>(color.b * 255.0f));
  return std::string(buffer);
}

/**
 * @brief Get theme preset name as string
 */
std::string ThemeManager::presetToString(ThemePreset preset) const {
  switch (preset) {
  case ThemePreset::Dark:
    return "Dark";
  case ThemePreset::Light:
    return "Light";
  case ThemePreset::HighContrast:
    return "HighContrast";
  case ThemePreset::Custom:
    return "Custom";
  default:
    return "Dark";
  }
}

/**
 * @brief Parse theme preset from string
 */
ThemePreset
ThemeManager::stringToPreset(const std::string &presetString) const {
  std::string str = presetString;

  // Convert to lowercase for case-insensitive comparison
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (str == "dark") {
    return ThemePreset::Dark;
  } else if (str == "light") {
    return ThemePreset::Light;
  } else if (str == "highcontrast" || str == "high_contrast") {
    return ThemePreset::HighContrast;
  } else if (str == "custom") {
    return ThemePreset::Custom;
  }

  return ThemePreset::Dark; // Default to Dark theme
}

} // namespace view
