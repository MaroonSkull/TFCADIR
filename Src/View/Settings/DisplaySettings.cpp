/**
 * @file DisplaySettings.cpp
 * @brief Implementation of the Display Settings dialog
 */

#include "DisplaySettings.hpp"

#include <spdlog/spdlog.h>

namespace view {
namespace settings {

DisplaySettings::DisplaySettings()
    : isOpen_(false), workingConfig_(), savedConfig_() {}

bool DisplaySettings::Render(DisplayConfig &config) {
  bool modified = false;

  if (!isOpen_) {
    return false;
  }

  // Set dialog size
  ImGui::SetNextWindowSize(ImVec2(450, 550), ImGuiCond_FirstUseEver);

  // Begin the dialog
  if (ImGui::Begin("Display Settings", &isOpen_, ImGuiWindowFlags_NoCollapse)) {
    // Store the working config at the start
    if (workingConfig_ != config) {
      workingConfig_ = config;
      savedConfig_ = config;
    }

    // Render sections
    if (RenderThemeSection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderColorSection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderFontSection()) {
      modified = true;
    }
    ImGui::Separator();
    if (RenderUISizeSection()) {
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

void DisplaySettings::Open() {
  isOpen_ = true;
  spdlog::info("Display Settings dialog opened");
}

void DisplaySettings::Close() {
  isOpen_ = false;
  spdlog::info("Display Settings dialog closed");
}

bool DisplaySettings::IsOpen() const { return isOpen_; }

bool DisplaySettings::HasChanged() const {
  return workingConfig_ != savedConfig_;
}

void DisplaySettings::SetOnApply(
    std::function<void(const DisplayConfig &)> callback) {
  onApply_ = std::move(callback);
}

void DisplaySettings::ApplyTheme(const DisplayConfig &config) {
  ImGuiStyle &style = ImGui::GetStyle();

  switch (config.theme) {
  case Theme::Dark:
    ApplyDarkTheme(style);
    break;
  case Theme::Light:
    ApplyLightTheme(style);
    break;
  case Theme::Custom:
    ApplyCustomColors(style, config);
    break;
  }

  spdlog::info("Applied {} theme", themeToString(config.theme));
}

void DisplaySettings::ResetToDefaults(DisplayConfig &config) {
  config = DisplayConfig();
  spdlog::info("Display settings reset to defaults");
}

std::vector<std::string> DisplaySettings::GetAvailableFonts() {
  return {"Default", "Roboto", "Arial", "DejaVu Sans", "Liberation Sans"};
}

bool DisplaySettings::RenderThemeSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen)) {
    int theme = static_cast<int>(workingConfig_.theme);

    if (ImGui::RadioButton("Dark", &theme, static_cast<int>(Theme::Dark))) {
      workingConfig_.theme = Theme::Dark;
      modified = true;
      spdlog::debug("Theme changed to Dark");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Dark theme with light text on dark background");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Light", &theme, static_cast<int>(Theme::Light))) {
      workingConfig_.theme = Theme::Light;
      modified = true;
      spdlog::debug("Theme changed to Light");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Light theme with dark text on light background");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Custom", &theme, static_cast<int>(Theme::Custom))) {
      workingConfig_.theme = Theme::Custom;
      modified = true;
      spdlog::debug("Theme changed to Custom");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Use custom colors defined below");
    }
  }

  return modified;
}

bool DisplaySettings::RenderColorSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Background color
    if (ImGui::ColorEdit4("Background Color", &workingConfig_.backgroundColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Background color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Canvas background color");
    }

    // Foreground color
    if (ImGui::ColorEdit4("Foreground Color", &workingConfig_.foregroundColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Foreground color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Default text and line color");
    }

    // Accent color
    if (ImGui::ColorEdit4("Accent Color", &workingConfig_.accentColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Accent color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Highlight and accent color for UI elements");
    }

    // Selection color
    if (ImGui::ColorEdit4("Selection Color", &workingConfig_.selectionColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Selection color changed");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Color for selected objects");
    }

    ImGui::Separator();
    ImGui::Text("Axis Colors:");

    // X-axis color
    if (ImGui::ColorEdit4("X-Axis##axis", &workingConfig_.axisXColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("X-axis color changed");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.42f, 0.42f, 1.0f), "X");

    // Y-axis color
    if (ImGui::ColorEdit4("Y-Axis##axis", &workingConfig_.axisYColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Y-axis color changed");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.31f, 0.80f, 0.77f, 1.0f), "Y");

    // Z-axis color
    if (ImGui::ColorEdit4("Z-Axis##axis", &workingConfig_.axisZColor.x,
                          ImGuiColorEditFlags_NoInputs)) {
      modified = true;
      spdlog::debug("Z-axis color changed");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.90f, 0.43f, 1.0f), "Z");
  }

  return modified;
}

bool DisplaySettings::RenderFontSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("Font", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Font selection
    ImGui::Text("Font:");
    ImGui::SameLine();

    auto fonts = GetAvailableFonts();
    if (ImGui::BeginCombo("##FontSelect", workingConfig_.fontName.c_str())) {
      for (const auto &font : fonts) {
        bool isSelected = (workingConfig_.fontName == font);
        if (ImGui::Selectable(font.c_str(), isSelected)) {
          workingConfig_.fontName = font;
          modified = true;
          spdlog::debug("Font changed to: {}", font);
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Select the UI font");
    }

    // Font size
    if (ImGui::SliderInt("Font Size", &workingConfig_.fontSize, 8, 24,
                         "%d pt")) {
      modified = true;
      spdlog::debug("Font size changed to: {}", workingConfig_.fontSize);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Font size in points (8-24)");
    }
  }

  return modified;
}

bool DisplaySettings::RenderUISizeSection() {
  bool modified = false;

  if (ImGui::CollapsingHeader("UI Size", ImGuiTreeNodeFlags_DefaultOpen)) {
    int uiSize = static_cast<int>(workingConfig_.uiSize);

    if (ImGui::RadioButton("Small", &uiSize, static_cast<int>(UISize::Small))) {
      workingConfig_.uiSize = UISize::Small;
      modified = true;
      spdlog::debug("UI size changed to Small");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Compact UI elements for more screen space");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Medium", &uiSize,
                           static_cast<int>(UISize::Medium))) {
      workingConfig_.uiSize = UISize::Medium;
      modified = true;
      spdlog::debug("UI size changed to Medium");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Standard UI element size");
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Large", &uiSize, static_cast<int>(UISize::Large))) {
      workingConfig_.uiSize = UISize::Large;
      modified = true;
      spdlog::debug("UI size changed to Large");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Larger UI elements for better accessibility");
    }
  }

  return modified;
}

bool DisplaySettings::RenderButtons(DisplayConfig &config) {
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
    ApplyTheme(config);
    if (onApply_) {
      onApply_(config);
    }
    spdlog::info("Display settings applied");
    shouldClose = true;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Apply changes and close dialog");
  }
  ImGui::SameLine();

  // Cancel button
  if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
    workingConfig_ = savedConfig_;
    spdlog::info("Display settings changes cancelled");
    shouldClose = true;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Discard changes and close dialog");
  }
  ImGui::SameLine();

  // Reset button
  if (ImGui::Button("Reset", ImVec2(buttonWidth, 0))) {
    ResetToDefaults(workingConfig_);
    spdlog::info("Display settings reset to defaults");
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Reset all settings to default values");
  }

  return shouldClose;
}

void DisplaySettings::ApplyDarkTheme(ImGuiStyle &style) {
  // Dark theme colors
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 0.75f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.60f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.58f, 0.78f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.48f, 0.68f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.10f, 0.10f, 0.10f, 0.25f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.48f, 0.68f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void DisplaySettings::ApplyLightTheme(ImGuiStyle &style) {
  // Light theme colors
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_Text] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
  colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.60f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.38f, 0.58f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_Tab] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
  colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.48f, 0.68f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.95f, 0.95f, 0.95f, 0.25f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.48f, 0.68f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.0f, 0.48f, 0.68f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.48f, 0.68f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void DisplaySettings::ApplyCustomColors(ImGuiStyle &style,
                                        const DisplayConfig &config) {
  // Start with dark theme as base
  ApplyDarkTheme(style);

  ImVec4 *colors = style.Colors;

  // Apply custom colors from config
  colors[ImGuiCol_WindowBg] =
      ImVec4(config.backgroundColor.x, config.backgroundColor.y,
             config.backgroundColor.z, config.backgroundColor.w);
  colors[ImGuiCol_Text] =
      ImVec4(config.foregroundColor.x, config.foregroundColor.y,
             config.foregroundColor.z, config.foregroundColor.w);
  colors[ImGuiCol_CheckMark] =
      ImVec4(config.accentColor.x, config.accentColor.y, config.accentColor.z,
             config.accentColor.w);
  colors[ImGuiCol_SliderGrab] =
      ImVec4(config.accentColor.x, config.accentColor.y, config.accentColor.z,
             config.accentColor.w);
  colors[ImGuiCol_SliderGrabActive] =
      ImVec4(config.accentColor.x * 1.2f, config.accentColor.y * 1.2f,
             config.accentColor.z * 1.2f, config.accentColor.w);
  colors[ImGuiCol_TextSelectedBg] =
      ImVec4(config.selectionColor.x, config.selectionColor.y,
             config.selectionColor.z, 0.35f);
  colors[ImGuiCol_NavHighlight] =
      ImVec4(config.accentColor.x, config.accentColor.y, config.accentColor.z,
             config.accentColor.w);
  colors[ImGuiCol_DockingPreview] = ImVec4(
      config.accentColor.x, config.accentColor.y, config.accentColor.z, 0.70f);
}

} // namespace settings
} // namespace view
