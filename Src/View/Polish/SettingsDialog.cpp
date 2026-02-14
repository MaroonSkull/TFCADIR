#include <View/Polish/SettingsDialog.hpp>
#include <View/Polish/ShortcutManager.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <yaml-cpp/yaml.h>

namespace view {

// Forward declaration for ThemeManager (will be implemented in future subtask)
class ThemeManager {
public:
  virtual ~ThemeManager() = default;
};

// ==========================================================================
// Constructor / Destructor
// ==========================================================================

SettingsDialog::SettingsDialog(UIFSMAdapter &fsmAdapter,
                               ShortcutManager &shortcutManager,
                               ThemeManager *themeManager)
    : fsmAdapter_(fsmAdapter), shortcutManager_(shortcutManager),
      themeManager_(themeManager) {
  // Initialize working settings with default values
  loadSettingsFromAdapter();
  originalSettings_ = workingSettings_;

  spdlog::info("SettingsDialog initialized");
}

SettingsDialog::~SettingsDialog() { spdlog::info("SettingsDialog destroyed"); }

// ==========================================================================
// Dialog Control
// ==========================================================================

void SettingsDialog::show() { visible_ = true; }

void SettingsDialog::hide() {
  visible_ = false;
  capturingShortcut_ = false;
  selectedShortcutIndex_ = -1;
}

bool SettingsDialog::isVisible() const { return visible_; }

void SettingsDialog::render() {
  if (!visible_) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
  if (ImGui::Begin("Settings", &visible_, windowFlags)) {
    // Tab bar for settings categories
    if (ImGui::BeginTabBar("SettingsTabBar")) {
      if (ImGui::BeginTabItem("General")) {
        renderGeneralTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Interface")) {
        renderInterfaceTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Input")) {
        renderInputTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("View")) {
        renderViewTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Performance")) {
        renderPerformanceTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Shortcuts")) {
        renderShortcutsTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Themes")) {
        renderThemesTab();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }

    // Bottom buttons
    ImGui::Separator();
    if (ImGui::BeginChild("BottomButtons", ImVec2(0, 0), false,
                          ImGuiWindowFlags_NoScrollbar)) {
      ImGui::Spacing();

      if (settingsModified_) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                           "* You have unsaved changes");
        ImGui::SameLine();
      }

      float buttonWidth = 120.0f;
      float spacing = 10.0f;

      // Calculate right-aligned button position
      float totalButtonWidth = buttonWidth * 3 + spacing * 2;
      float buttonX = ImGui::GetContentRegionAvail().x - totalButtonWidth;

      ImGui::SetCursorPosX(buttonX);

      if (ImGui::Button("Apply", ImVec2(buttonWidth, 0))) {
        apply();
      }
      ImGui::SameLine(0.0f, spacing);

      if (ImGui::Button("Reset", ImVec2(buttonWidth, 0))) {
        reset();
      }
      ImGui::SameLine(0.0f, spacing);

      if (ImGui::Button("Close", ImVec2(buttonWidth, 0))) {
        hide();
      }
    }
    ImGui::EndChild();
  }
  ImGui::End();
}

// ==========================================================================
// Settings Operations
// ==========================================================================

void SettingsDialog::apply() {
  applySettingsToAdapter();
  originalSettings_ = workingSettings_;
  settingsModified_ = false;
  spdlog::info("Settings applied");
}

void SettingsDialog::reset() {
  workingSettings_ = originalSettings_;
  settingsModified_ = false;
  spdlog::info("Settings reset to original values");
}

bool SettingsDialog::saveToFile(const std::string &filePath) {
  try {
    YAML::Node root;
    YAML::Node settingsNode;

    // General settings
    YAML::Node general;
    general["default_mode"] = workingSettings_.general.defaultMode;
    general["show_welcome_screen"] = workingSettings_.general.showWelcomeScreen;
    general["auto_save_interval"] = workingSettings_.general.autoSaveInterval;
    general["backup_count"] = workingSettings_.general.backupCount;
    general["units"] = workingSettings_.general.units;
    general["check_updates"] = workingSettings_.general.checkUpdates;
    settingsNode["general"] = general;

    // Interface settings
    YAML::Node interface;
    interface["theme"] = workingSettings_.interface.theme;
    interface["icon_size"] = workingSettings_.interface.iconSize;
    interface["font"] = workingSettings_.interface.font;
    interface["show_tooltips"] = workingSettings_.interface.showTooltips;
    interface["tooltip_delay"] = workingSettings_.interface.tooltipDelay;
    interface["remember_window"] = workingSettings_.interface.rememberWindow;
    interface["remember_layout"] = workingSettings_.interface.rememberLayout;
    interface["panel_width"] = workingSettings_.interface.panelWidth;
    interface["startup_panels"] = workingSettings_.interface.startupPanels;
    settingsNode["interface"] = interface;

    // Input settings
    YAML::Node input;
    input["orbit_sensitivity"] = workingSettings_.input.orbitSensitivity;
    input["zoom_sensitivity"] = workingSettings_.input.zoomSensitivity;
    input["pan_speed"] = workingSettings_.input.panSpeed;
    input["scroll_zoom_to_cursor"] = workingSettings_.input.scrollZoomToCursor;
    input["double_click_action"] = workingSettings_.input.doubleClickAction;
    input["tablet_support"] = workingSettings_.input.tabletSupport;
    input["pressure_sensitivity"] = workingSettings_.input.pressureSensitivity;
    input["input_repeat_delay"] = workingSettings_.input.inputRepeatDelay;
    input["input_repeat_rate"] = workingSettings_.input.inputRepeatRate;
    settingsNode["input"] = input;

    // View settings
    YAML::Node view;
    view["show_grid"] = workingSettings_.view.showGrid;
    view["show_minor_grid"] = workingSettings_.view.showMinorGrid;
    view["show_axis"] = workingSettings_.view.showAxis;
    view["show_origin"] = workingSettings_.view.showOrigin;
    view["grid_spacing_major"] = workingSettings_.view.gridSpacingMajor;
    view["grid_spacing_minor"] = workingSettings_.view.gridSpacingMinor;
    view["grid_color"] = workingSettings_.view.gridColor;
    view["grid_opacity"] = workingSettings_.view.gridOpacity;
    settingsNode["view"] = view;

    // Performance settings
    YAML::Node performance;
    performance["vsync"] = workingSettings_.performance.vsync;
    performance["max_fps"] = workingSettings_.performance.maxFPS;
    performance["anti_aliasing"] = workingSettings_.performance.antiAliasing;
    performance["anisotropic_filtering"] =
        workingSettings_.performance.anisotropicFiltering;
    performance["render_distance"] =
        workingSettings_.performance.renderDistance;
    performance["shadow_quality"] = workingSettings_.performance.shadowQuality;
    performance["effects_quality"] =
        workingSettings_.performance.effectsQuality;
    settingsNode["performance"] = performance;

    // Shortcuts
    YAML::Node shortcuts;
    for (const auto &entry : workingSettings_.shortcuts) {
      YAML::Node shortcut;
      shortcut["action"] = entry.action;
      shortcut["shortcut"] = entry.shortcut;
      shortcuts.push_back(shortcut);
    }
    settingsNode["shortcuts"] = shortcuts;

    // Themes
    YAML::Node themes;
    themes["current"] = workingSettings_.theme.currentTheme;
    YAML::Node colors;
    colors["background"] = workingSettings_.theme.customColors.background;
    colors["foreground"] = workingSettings_.theme.customColors.foreground;
    colors["accent"] = workingSettings_.theme.customColors.accent;
    colors["border"] = workingSettings_.theme.customColors.border;
    colors["header"] = workingSettings_.theme.customColors.header;
    colors["selection"] = workingSettings_.theme.customColors.selection;
    themes["custom_colors"] = colors;
    settingsNode["themes"] = themes;

    root["settings"] = settingsNode;

    std::ofstream outFile(filePath);
    outFile << root;
    outFile.close();

    spdlog::info("Settings saved to {}", filePath);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to save settings: {}", e.what());
    return false;
  }
}

bool SettingsDialog::loadFromFile(const std::string &filePath) {
  try {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
      spdlog::warn("Settings file not found: {}", filePath);
      return false;
    }

    YAML::Node root = YAML::Load(inFile);
    inFile.close();

    if (!root["settings"]) {
      spdlog::warn("Invalid settings file format");
      return false;
    }

    YAML::Node settings = root["settings"];

    // Load general settings
    if (settings["general"]) {
      YAML::Node general = settings["general"];
      workingSettings_.general.defaultMode =
          general["default_mode"].as<std::string>("2D");
      workingSettings_.general.showWelcomeScreen =
          general["show_welcome_screen"].as<bool>(true);
      workingSettings_.general.autoSaveInterval =
          general["auto_save_interval"].as<int>(5);
      workingSettings_.general.backupCount = general["backup_count"].as<int>(3);
      workingSettings_.general.units = general["units"].as<std::string>("mm");
      workingSettings_.general.checkUpdates =
          general["check_updates"].as<bool>(true);
    }

    // Load interface settings
    if (settings["interface"]) {
      YAML::Node interface = settings["interface"];
      workingSettings_.interface.theme =
          interface["theme"].as<std::string>("Dark");
      workingSettings_.interface.iconSize =
          interface["icon_size"].as<std::string>("Medium");
      workingSettings_.interface.font =
          interface["font"].as<std::string>("Default");
      workingSettings_.interface.showTooltips =
          interface["show_tooltips"].as<bool>(true);
      workingSettings_.interface.tooltipDelay =
          interface["tooltip_delay"].as<int>(500);
      workingSettings_.interface.rememberWindow =
          interface["remember_window"].as<bool>(true);
      workingSettings_.interface.rememberLayout =
          interface["remember_layout"].as<bool>(true);
      workingSettings_.interface.panelWidth =
          interface["panel_width"].as<int>(280);
      if (interface["startup_panels"]) {
        workingSettings_.interface.startupPanels =
            interface["startup_panels"].as<std::vector<std::string>>();
      }
    }

    // Load input settings
    if (settings["input"]) {
      YAML::Node input = settings["input"];
      workingSettings_.input.orbitSensitivity =
          input["orbit_sensitivity"].as<float>(0.5f);
      workingSettings_.input.zoomSensitivity =
          input["zoom_sensitivity"].as<float>(0.6f);
      workingSettings_.input.panSpeed = input["pan_speed"].as<float>(0.5f);
      workingSettings_.input.scrollZoomToCursor =
          input["scroll_zoom_to_cursor"].as<bool>(true);
      workingSettings_.input.doubleClickAction =
          input["double_click_action"].as<std::string>("FitToView");
      workingSettings_.input.tabletSupport =
          input["tablet_support"].as<bool>(false);
      workingSettings_.input.pressureSensitivity =
          input["pressure_sensitivity"].as<float>(0.7f);
      workingSettings_.input.inputRepeatDelay =
          input["input_repeat_delay"].as<int>(500);
      workingSettings_.input.inputRepeatRate =
          input["input_repeat_rate"].as<int>(20);
    }

    // Load view settings
    if (settings["view"]) {
      YAML::Node view = settings["view"];
      workingSettings_.view.showGrid = view["show_grid"].as<bool>(true);
      workingSettings_.view.showMinorGrid =
          view["show_minor_grid"].as<bool>(false);
      workingSettings_.view.showAxis = view["show_axis"].as<bool>(true);
      workingSettings_.view.showOrigin = view["show_origin"].as<bool>(false);
      workingSettings_.view.gridSpacingMajor =
          view["grid_spacing_major"].as<float>(10.0f);
      workingSettings_.view.gridSpacingMinor =
          view["grid_spacing_minor"].as<int>(10);
      workingSettings_.view.gridColor =
          view["grid_color"].as<std::string>("#444444");
      workingSettings_.view.gridOpacity = view["grid_opacity"].as<float>(0.8f);
    }

    // Load performance settings
    if (settings["performance"]) {
      YAML::Node performance = settings["performance"];
      workingSettings_.performance.vsync = performance["vsync"].as<bool>(true);
      workingSettings_.performance.maxFPS = performance["max_fps"].as<int>(60);
      workingSettings_.performance.antiAliasing =
          performance["anti_aliasing"].as<int>(4);
      workingSettings_.performance.anisotropicFiltering =
          performance["anisotropic_filtering"].as<int>(16);
      workingSettings_.performance.renderDistance =
          performance["render_distance"].as<int>(1000);
      workingSettings_.performance.shadowQuality =
          performance["shadow_quality"].as<std::string>("Medium");
      workingSettings_.performance.effectsQuality =
          performance["effects_quality"].as<std::string>("High");
    }

    // Load shortcuts
    if (settings["shortcuts"]) {
      workingSettings_.shortcuts.clear();
      for (const auto &shortcutNode : settings["shortcuts"]) {
        ShortcutEntry entry;
        entry.action = shortcutNode["action"].as<std::string>();
        entry.shortcut = shortcutNode["shortcut"].as<std::string>();
        entry.description = ""; // Will be filled from ShortcutManager
        workingSettings_.shortcuts.push_back(entry);
      }
    }

    // Load themes
    if (settings["themes"]) {
      YAML::Node themes = settings["themes"];
      workingSettings_.theme.currentTheme =
          themes["current"].as<std::string>("Dark");
      if (themes["custom_colors"]) {
        YAML::Node colors = themes["custom_colors"];
        workingSettings_.theme.customColors.background =
            colors["background"].as<std::string>("#1E1E1E");
        workingSettings_.theme.customColors.foreground =
            colors["foreground"].as<std::string>("#D4D4D4");
        workingSettings_.theme.customColors.accent =
            colors["accent"].as<std::string>("#007ACC");
        workingSettings_.theme.customColors.border =
            colors["border"].as<std::string>("#3E3E42");
        workingSettings_.theme.customColors.header =
            colors["header"].as<std::string>("#2D2D30");
        workingSettings_.theme.customColors.selection =
            colors["selection"].as<std::string>("#264F78");
      }
    }

    spdlog::info("Settings loaded from {}", filePath);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to load settings: {}", e.what());
    return false;
  }
}

// ==========================================================================
// Tab Rendering Methods
// ==========================================================================

void SettingsDialog::renderGeneralTab() {
  ImGui::Text("General Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // Default mode
  const char *modes[] = {"2D", "3D"};
  int currentMode = workingSettings_.general.defaultMode == "3D" ? 1 : 0;
  if (ImGui::Combo("Default Mode", &currentMode, modes, 2)) {
    workingSettings_.general.defaultMode = modes[currentMode];
    markModified();
  }

  // Show welcome screen
  if (ImGui::Checkbox("Show Welcome Screen",
                      &workingSettings_.general.showWelcomeScreen)) {
    markModified();
  }

  // Auto-save interval
  if (ImGui::SliderInt("Auto-save Interval (minutes)",
                       &workingSettings_.general.autoSaveInterval, 0, 60)) {
    markModified();
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Set to 0 to disable auto-save");
  }

  // Backup count
  if (ImGui::SliderInt("Backup Count", &workingSettings_.general.backupCount, 0,
                       10)) {
    markModified();
  }

  // Units
  const char *units[] = {"mm", "inches"};
  int currentUnit = workingSettings_.general.units == "inches" ? 1 : 0;
  if (ImGui::Combo("Units", &currentUnit, units, 2)) {
    workingSettings_.general.units = units[currentUnit];
    markModified();
  }

  // Check for updates
  if (ImGui::Checkbox("Check for Updates Automatically",
                      &workingSettings_.general.checkUpdates)) {
    markModified();
  }
}

void SettingsDialog::renderInterfaceTab() {
  ImGui::Text("Interface Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // Theme selection
  const char *themes[] = {"Dark", "Light", "High Contrast"};
  int currentThemeIndex = 0;
  if (workingSettings_.interface.theme == "Light")
    currentThemeIndex = 1;
  else if (workingSettings_.interface.theme == "High Contrast")
    currentThemeIndex = 2;

  if (ImGui::Combo("Theme", &currentThemeIndex, themes, 3)) {
    workingSettings_.interface.theme = themes[currentThemeIndex];
    markModified();
  }

  // Icon size
  const char *iconSizes[] = {"Small", "Medium", "Large"};
  int currentIconSize = 1;
  if (workingSettings_.interface.iconSize == "Small")
    currentIconSize = 0;
  else if (workingSettings_.interface.iconSize == "Large")
    currentIconSize = 2;

  if (ImGui::Combo("Icon Size", &currentIconSize, iconSizes, 3)) {
    workingSettings_.interface.iconSize = iconSizes[currentIconSize];
    markModified();
  }

  // Font selection
  char fontBuffer[256];
  strncpy(fontBuffer, workingSettings_.interface.font.c_str(),
          sizeof(fontBuffer) - 1);
  fontBuffer[sizeof(fontBuffer) - 1] = '\0';
  if (ImGui::InputText("Font", fontBuffer, sizeof(fontBuffer))) {
    workingSettings_.interface.font = fontBuffer;
    markModified();
  }

  // Show tooltips
  if (ImGui::Checkbox("Show Tooltips",
                      &workingSettings_.interface.showTooltips)) {
    markModified();
  }

  // Tooltip delay
  if (ImGui::SliderInt("Tooltip Delay (ms)",
                       &workingSettings_.interface.tooltipDelay, 0, 2000)) {
    markModified();
  }

  // Remember window position
  if (ImGui::Checkbox("Remember Window Position",
                      &workingSettings_.interface.rememberWindow)) {
    markModified();
  }

  // Remember panel layout
  if (ImGui::Checkbox("Remember Panel Layout",
                      &workingSettings_.interface.rememberLayout)) {
    markModified();
  }

  // Panel width
  if (ImGui::SliderInt("Panel Width", &workingSettings_.interface.panelWidth,
                       200, 500)) {
    markModified();
  }

  // Startup panels
  ImGui::Text("Startup Panels:");
  for (size_t i = 0; i < workingSettings_.interface.startupPanels.size(); ++i) {
    std::string label = "##panel" + std::to_string(i);
    char panelBuffer[256];
    strncpy(panelBuffer, workingSettings_.interface.startupPanels[i].c_str(),
            sizeof(panelBuffer) - 1);
    panelBuffer[sizeof(panelBuffer) - 1] = '\0';
    ImGui::InputText(label.c_str(), panelBuffer, sizeof(panelBuffer));
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      markModified();
    }
  }
}

void SettingsDialog::renderInputTab() {
  ImGui::Text("Input Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // Orbit sensitivity
  if (ImGui::SliderFloat("Orbit Sensitivity",
                         &workingSettings_.input.orbitSensitivity, 0.0f,
                         1.0f)) {
    markModified();
  }

  // Zoom sensitivity
  if (ImGui::SliderFloat("Zoom Sensitivity",
                         &workingSettings_.input.zoomSensitivity, 0.0f, 1.0f)) {
    markModified();
  }

  // Pan speed
  if (ImGui::SliderFloat("Pan Speed", &workingSettings_.input.panSpeed, 0.0f,
                         1.0f)) {
    markModified();
  }

  // Scroll zoom to cursor
  if (ImGui::Checkbox("Scroll Wheel Zooms to Cursor",
                      &workingSettings_.input.scrollZoomToCursor)) {
    markModified();
  }

  // Double-click action
  const char *actions[] = {"FitToView", "Edit", "None"};
  int currentAction = 0;
  if (workingSettings_.input.doubleClickAction == "Edit")
    currentAction = 1;
  else if (workingSettings_.input.doubleClickAction == "None")
    currentAction = 2;

  if (ImGui::Combo("Double-Click Action", &currentAction, actions, 3)) {
    workingSettings_.input.doubleClickAction = actions[currentAction];
    markModified();
  }

  // Tablet support
  if (ImGui::Checkbox("Enable Tablet/Stylus Support",
                      &workingSettings_.input.tabletSupport)) {
    markModified();
  }

  // Pressure sensitivity
  if (ImGui::SliderFloat("Pressure Sensitivity",
                         &workingSettings_.input.pressureSensitivity, 0.0f,
                         1.0f)) {
    markModified();
  }

  // Input repeat delay
  if (ImGui::SliderInt("Input Repeat Delay (ms)",
                       &workingSettings_.input.inputRepeatDelay, 100, 1000)) {
    markModified();
  }

  // Input repeat rate
  if (ImGui::SliderInt("Input Repeat Rate (per second)",
                       &workingSettings_.input.inputRepeatRate, 1, 50)) {
    markModified();
  }
}

void SettingsDialog::renderViewTab() {
  ImGui::Text("View Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // Show grid
  if (ImGui::Checkbox("Show Grid", &workingSettings_.view.showGrid)) {
    markModified();
  }

  // Show minor grid
  if (ImGui::Checkbox("Show Minor Grid Lines",
                      &workingSettings_.view.showMinorGrid)) {
    markModified();
  }

  // Show axis
  if (ImGui::Checkbox("Show Axis", &workingSettings_.view.showAxis)) {
    markModified();
  }

  // Show origin
  if (ImGui::Checkbox("Show Origin", &workingSettings_.view.showOrigin)) {
    markModified();
  }

  // Grid spacing major
  if (ImGui::SliderFloat("Grid Spacing (Major)",
                         &workingSettings_.view.gridSpacingMajor, 1.0f,
                         100.0f)) {
    markModified();
  }

  // Grid spacing minor
  if (ImGui::SliderInt("Minor Divisions",
                       &workingSettings_.view.gridSpacingMinor, 2, 20)) {
    markModified();
  }

  // Grid color
  ImGui::Text("Grid Color:");
  renderColorPicker("##gridcolor", workingSettings_.view.gridColor);

  // Grid opacity
  if (ImGui::SliderFloat("Grid Opacity", &workingSettings_.view.gridOpacity,
                         0.0f, 1.0f)) {
    markModified();
  }
}

void SettingsDialog::renderPerformanceTab() {
  ImGui::Text("Performance Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // VSync
  if (ImGui::Checkbox("Enable VSync", &workingSettings_.performance.vsync)) {
    markModified();
  }

  // Max FPS
  if (ImGui::SliderInt("Maximum FPS", &workingSettings_.performance.maxFPS, 30,
                       144)) {
    markModified();
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Set to 0 for unlimited FPS");
  }

  // Anti-aliasing
  const char *aaLevels[] = {"Off", "2x", "4x", "8x"};
  int currentAA = 0;
  if (workingSettings_.performance.antiAliasing == 2)
    currentAA = 1;
  else if (workingSettings_.performance.antiAliasing == 4)
    currentAA = 2;
  else if (workingSettings_.performance.antiAliasing == 8)
    currentAA = 3;

  if (ImGui::Combo("Anti-Aliasing", &currentAA, aaLevels, 4)) {
    workingSettings_.performance.antiAliasing =
        currentAA == 0 ? 0 : (2 << (currentAA - 1));
    markModified();
  }

  // Anisotropic filtering
  const char *afLevels[] = {"Off", "2x", "4x", "8x", "16x"};
  int currentAF = 0;
  if (workingSettings_.performance.anisotropicFiltering == 2)
    currentAF = 1;
  else if (workingSettings_.performance.anisotropicFiltering == 4)
    currentAF = 2;
  else if (workingSettings_.performance.anisotropicFiltering == 8)
    currentAF = 3;
  else if (workingSettings_.performance.anisotropicFiltering == 16)
    currentAF = 4;

  if (ImGui::Combo("Anisotropic Filtering", &currentAF, afLevels, 5)) {
    workingSettings_.performance.anisotropicFiltering =
        currentAF == 0 ? 0 : (2 << (currentAF - 1));
    markModified();
  }

  // Render distance
  if (ImGui::SliderInt("Render Distance",
                       &workingSettings_.performance.renderDistance, 100,
                       5000)) {
    markModified();
  }

  // Shadow quality
  const char *shadowQualities[] = {"Low", "Medium", "High", "Ultra"};
  int currentShadow = 1;
  if (workingSettings_.performance.shadowQuality == "Low")
    currentShadow = 0;
  else if (workingSettings_.performance.shadowQuality == "High")
    currentShadow = 2;
  else if (workingSettings_.performance.shadowQuality == "Ultra")
    currentShadow = 3;

  if (ImGui::Combo("Shadow Quality", &currentShadow, shadowQualities, 4)) {
    workingSettings_.performance.shadowQuality = shadowQualities[currentShadow];
    markModified();
  }

  // Effects quality
  const char *effectsQualities[] = {"Low", "Medium", "High", "Ultra"};
  int currentEffects = 2;
  if (workingSettings_.performance.effectsQuality == "Low")
    currentEffects = 0;
  else if (workingSettings_.performance.effectsQuality == "Medium")
    currentEffects = 1;
  else if (workingSettings_.performance.effectsQuality == "Ultra")
    currentEffects = 3;

  if (ImGui::Combo("Effects Quality", &currentEffects, effectsQualities, 4)) {
    workingSettings_.performance.effectsQuality =
        effectsQualities[currentEffects];
    markModified();
  }
}

void SettingsDialog::renderShortcutsTab() {
  ImGui::Text("Keyboard Shortcuts");
  ImGui::Separator();
  ImGui::Spacing();

  // Search/filter
  ImGui::Text("Search Shortcuts:");
  ImGui::InputText("##shortcutfilter", shortcutFilter_,
                   sizeof(shortcutFilter_));

  ImGui::Spacing();

  // Shortcuts list
  if (ImGui::BeginChild("ShortcutsList", ImVec2(0, -50), true)) {
    auto filteredShortcuts = getFilteredShortcuts();

    for (size_t i = 0; i < filteredShortcuts.size(); ++i) {
      const auto &entry = filteredShortcuts[i];

      ImGui::PushID(static_cast<int>(i));

      // Action name
      ImGui::Text("%s", entry.action.c_str());
      ImGui::SameLine(200);

      // Current shortcut
      ImGui::Text("%s", entry.shortcut.c_str());
      ImGui::SameLine();

      // Set button
      if (ImGui::Button("Set")) {
        selectedShortcutIndex_ = static_cast<int>(i);
        capturingShortcut_ = true;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();

      // Clear button
      if (ImGui::Button("Clear")) {
        // Find and clear the shortcut
        for (auto &s : workingSettings_.shortcuts) {
          if (s.action == entry.action) {
            s.shortcut = "";
            markModified();
            break;
          }
        }
      }

      ImGui::PopID();
    }
  }
  ImGui::EndChild();

  // Capture shortcut popup
  if (capturingShortcut_ && selectedShortcutIndex_ >= 0) {
    ImGui::OpenPopup("CaptureShortcut");
    if (ImGui::BeginPopupModal("Capture Shortcut", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Press a key combination...");
      ImGui::Text("(Press ESC to cancel)");

      // Check for key input
      ImGuiIO &io = ImGui::GetIO();
      if (io.KeyCtrl || io.KeyShift || io.KeyAlt) {
        std::string combo;
        if (io.KeyCtrl)
          combo += "Ctrl+";
        if (io.KeyShift)
          combo += "Shift+";
        if (io.KeyAlt)
          combo += "Alt+";

        // Find pressed key
        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END;
             ++key) {
          if (ImGui::IsKeyDown(static_cast<ImGuiKey>(key)) && !io.KeyCtrl &&
              !io.KeyShift && !io.KeyAlt) {
            combo += ImGui::GetKeyName(static_cast<ImGuiKey>(key));

            // Update the shortcut
            auto filteredShortcuts = getFilteredShortcuts();
            if (selectedShortcutIndex_ <
                static_cast<int>(filteredShortcuts.size())) {
              const std::string &action =
                  filteredShortcuts[selectedShortcutIndex_].action;
              for (auto &s : workingSettings_.shortcuts) {
                if (s.action == action) {
                  s.shortcut = combo;
                  markModified();
                  break;
                }
              }
            }

            capturingShortcut_ = false;
            selectedShortcutIndex_ = -1;
            ImGui::CloseCurrentPopup();
            break;
          }
        }
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        capturingShortcut_ = false;
        selectedShortcutIndex_ = -1;
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
  }

  // Reset to defaults button
  if (ImGui::Button("Reset to Defaults")) {
    // Reload from ShortcutManager
    auto allShortcuts = shortcutManager_.getAllShortcuts();
    workingSettings_.shortcuts.clear();
    for (const auto &sc : allShortcuts) {
      ShortcutEntry entry;
      entry.action = sc.id;
      entry.shortcut = ""; // Will be filled by keyComboToString
      entry.description = sc.description;
      workingSettings_.shortcuts.push_back(entry);
    }
    markModified();
  }
}

void SettingsDialog::renderThemesTab() {
  ImGui::Text("Theme Settings");
  ImGui::Separator();
  ImGui::Spacing();

  // Theme presets
  auto themePresets = getThemePresets();
  std::vector<const char *> presetItems;
  presetItems.reserve(themePresets.size());
  for (const auto &preset : themePresets) {
    presetItems.push_back(preset.c_str());
  }

  if (ImGui::Combo("Theme Preset", &selectedThemePreset_, presetItems.data(),
                   static_cast<int>(themePresets.size()))) {
    applyThemePreset(themePresets[selectedThemePreset_]);
    markModified();
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Text("Custom Colors");
  ImGui::Spacing();

  // Color pickers
  renderColorPicker("Background",
                    workingSettings_.theme.customColors.background);
  renderColorPicker("Foreground",
                    workingSettings_.theme.customColors.foreground);
  renderColorPicker("Accent", workingSettings_.theme.customColors.accent);
  renderColorPicker("Border", workingSettings_.theme.customColors.border);
  renderColorPicker("Header", workingSettings_.theme.customColors.header);
  renderColorPicker("Selection", workingSettings_.theme.customColors.selection);

  ImGui::Spacing();

  if (ImGui::Button("Apply Custom Theme")) {
    workingSettings_.theme.isCustom = true;
    markModified();
  }
}

// ==========================================================================
// Helper Methods
// ==========================================================================

void SettingsDialog::loadSettingsFromAdapter() {
  // Query UIFSMAdapter for current settings
  // Note: Most settings structures don't exist in UIFSMAdapter yet,
  // so we use default values for now

  // Grid settings exist in UIFSMAdapter
  auto gridSettings = fsmAdapter_.getGridSettings();
  workingSettings_.view.showGrid = gridSettings.visible;
  workingSettings_.view.showMinorGrid = gridSettings.showMinorLines;
  workingSettings_.view.showAxis = gridSettings.showAxes;
  workingSettings_.view.showOrigin = gridSettings.showOrigin;
  workingSettings_.view.gridSpacingMajor = gridSettings.majorSpacing;
  workingSettings_.view.gridSpacingMinor = gridSettings.minorDivisions;

  // Load shortcuts from ShortcutManager
  auto allShortcuts = shortcutManager_.getAllShortcuts();
  workingSettings_.shortcuts.clear();
  for (const auto &sc : allShortcuts) {
    ShortcutEntry entry;
    entry.action = sc.id;
    entry.shortcut = ""; // Will be filled by keyComboToString
    entry.description = sc.description;
    workingSettings_.shortcuts.push_back(entry);
  }
}

void SettingsDialog::applySettingsToAdapter() {
  // Apply grid settings to UIFSMAdapter
  GridSettings gridSettings;
  gridSettings.visible = workingSettings_.view.showGrid;
  gridSettings.showMinorLines = workingSettings_.view.showMinorGrid;
  gridSettings.showAxes = workingSettings_.view.showAxis;
  gridSettings.showOrigin = workingSettings_.view.showOrigin;
  gridSettings.majorSpacing = workingSettings_.view.gridSpacingMajor;
  gridSettings.minorDivisions = workingSettings_.view.gridSpacingMinor;
  fsmAdapter_.setGridSettings(gridSettings);

  // Apply shortcuts to ShortcutManager
  // Note: This would require ShortcutManager to have importShortcuts method
  // For now, shortcuts are managed separately

  spdlog::info("Settings applied to UIFSMAdapter");
}

void SettingsDialog::markModified() { settingsModified_ = true; }

ImVec4 SettingsDialog::hexToColor(const std::string &hex) const {
  ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f);

  if (hex.empty() || hex[0] != '#') {
    return color;
  }

  std::string hexValue = hex.substr(1);
  if (hexValue.length() != 6) {
    return color;
  }

  try {
    unsigned int value = std::stoul(hexValue, nullptr, 16);
    color.x = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
    color.y = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
    color.z = static_cast<float>(value & 0xFF) / 255.0f;
    color.w = 1.0f;
  } catch (...) {
    // Invalid hex format, return default white
  }

  return color;
}

std::string SettingsDialog::colorToHex(const ImVec4 &color) const {
  int r = static_cast<int>(color.x * 255.0f);
  int g = static_cast<int>(color.y * 255.0f);
  int b = static_cast<int>(color.z * 255.0f);

  char hex[8];
  snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
  return std::string(hex);
}

void SettingsDialog::renderColorPicker(const char *label,
                                       std::string &colorHex) {
  ImGui::PushID(label);

  ImVec4 color = hexToColor(colorHex);
  if (ImGui::ColorEdit4(label, &color.x,
                        ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_NoLabel |
                            ImGuiColorEditFlags_PickerHueWheel)) {
    colorHex = colorToHex(color);
    markModified();
  }

  ImGui::PopID();
}

std::vector<ShortcutEntry> SettingsDialog::getFilteredShortcuts() const {
  std::vector<ShortcutEntry> filtered;

  std::string filter(shortcutFilter_);
  // Convert filter to lowercase for case-insensitive search
  std::transform(filter.begin(), filter.end(), filter.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  for (const auto &entry : workingSettings_.shortcuts) {
    std::string actionLower = entry.action;
    std::transform(actionLower.begin(), actionLower.end(), actionLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (filter.empty() || actionLower.find(filter) != std::string::npos) {
      filtered.push_back(entry);
    }
  }

  return filtered;
}

std::vector<std::string> SettingsDialog::getThemePresets() const {
  return {"Dark", "Light", "High Contrast", "Custom"};
}

void SettingsDialog::applyThemePreset(const std::string &presetName) {
  if (presetName == "Dark") {
    workingSettings_.theme.customColors.background = "#1E1E1E";
    workingSettings_.theme.customColors.foreground = "#D4D4D4";
    workingSettings_.theme.customColors.accent = "#007ACC";
    workingSettings_.theme.customColors.border = "#3E3E42";
    workingSettings_.theme.customColors.header = "#2D2D30";
    workingSettings_.theme.customColors.selection = "#264F78";
  } else if (presetName == "Light") {
    workingSettings_.theme.customColors.background = "#FFFFFF";
    workingSettings_.theme.customColors.foreground = "#000000";
    workingSettings_.theme.customColors.accent = "#0066CC";
    workingSettings_.theme.customColors.border = "#CCCCCC";
    workingSettings_.theme.customColors.header = "#F0F0F0";
    workingSettings_.theme.customColors.selection = "#B4D8FD";
  } else if (presetName == "High Contrast") {
    workingSettings_.theme.customColors.background = "#000000";
    workingSettings_.theme.customColors.foreground = "#FFFFFF";
    workingSettings_.theme.customColors.accent = "#FFFF00";
    workingSettings_.theme.customColors.border = "#FFFFFF";
    workingSettings_.theme.customColors.header = "#000000";
    workingSettings_.theme.customColors.selection = "#FFFFFF";
  }
  // Custom preset doesn't change colors

  workingSettings_.theme.currentTheme = presetName;
  markModified();
}

} // namespace view
