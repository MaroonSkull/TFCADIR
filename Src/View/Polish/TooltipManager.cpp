#include "TooltipManager.hpp"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <unordered_map>

namespace view {

// ImGui key code to readable name mapping
static const std::unordered_map<int, std::string> KEY_CODE_NAMES = {
    {65, "A"},
    {66, "B"},
    {67, "C"},
    {68, "D"},
    {69, "E"},
    {70, "F"},
    {71, "G"},
    {72, "H"},
    {73, "I"},
    {74, "J"},
    {75, "K"},
    {76, "L"},
    {77, "M"},
    {78, "N"},
    {79, "O"},
    {80, "P"},
    {81, "Q"},
    {82, "R"},
    {83, "S"},
    {84, "T"},
    {85, "U"},
    {86, "V"},
    {87, "W"},
    {88, "X"},
    {89, "Y"},
    {90, "Z"},
    {48, "0"},
    {49, "1"},
    {50, "2"},
    {51, "3"},
    {52, "4"},
    {53, "5"},
    {54, "6"},
    {55, "7"},
    {56, "8"},
    {57, "9"},
    {32, "Space"},
    {257, "Insert"},
    {259, "Delete"},
    {261, "Escape"},
    {262, "Enter"},
    {263, "Tab"},
    {264, "Backspace"},
    {256, "KP_0"},
    {257, "KP_1"},
    {258, "KP_2"},
    {259, "KP_3"},
    {260, "KP_4"},
    {261, "KP_5"},
    {262, "KP_6"},
    {263, "KP_7"},
    {264, "KP_8"},
    {265, "KP_9"},
    {266, "KP_Decimal"},
    {267, "KP_Divide"},
    {268, "KP_Multiply"},
    {269, "KP_Subtract"},
    {270, "KP_Add"},
    {271, "KP_Enter"},
    {272, "KP_Equals"},
};

/**
 * @brief Get readable key name from ImGui key code
 * @param keyCode The ImGui key code
 * @return Readable key name (e.g., "Z" for key code 90)
 */
static std::string getKeyName(int keyCode) {
  auto it = KEY_CODE_NAMES.find(keyCode);
  if (it != KEY_CODE_NAMES.end()) {
    return it->second;
  }
  return "Key" + std::to_string(keyCode);
}

/**
 * @brief Construct a TooltipManager
 * @param fsmAdapter Reference to the UIFSMAdapter for state access
 */
TooltipManager::TooltipManager(UIFSMAdapter &fsmAdapter)
    : fsmAdapter_(fsmAdapter) {
  // Initialize cached settings with defaults
  cachedSettings_.enabled = true;
  cachedSettings_.delay = 500;
  cachedSettings_.duration = 0;
  cachedSettings_.maxWidth = 400;
  cachedSettings_.position = TooltipPosition::FollowMouse;
  cachedSettings_.showShortcuts = true;
  cachedSettings_.showDescriptions = true;
  cachedSettings_.wrapText = true;
  cachedSettings_.dirty = true;

  // Register callback for tooltip settings changes to invalidate cache
  fsmAdapter_.setTooltipSettingsChangedCallback([this]() {
    markCacheDirty();
    spdlog::debug("TooltipManager cache invalidated due to settings change");
  });

  spdlog::debug("TooltipManager initialized");
}

/**
 * @brief Destructor
 */
TooltipManager::~TooltipManager() = default;

// ==========================================================================
// Tooltip Display Methods
// ==========================================================================

/**
 * @brief Show a tooltip for a tool button
 */
void TooltipManager::showToolTooltip(const std::string &toolId,
                                     const std::string &title,
                                     const std::string &description) {
  TooltipContent content;
  content.title = title;
  content.description = description;
  content.type = TooltipType::Tool;
  content.shortcut = getToolShortcut(toolId);

  // Note: Actual tooltip display is handled by the UI layer (ImGUI)
  // This method only formats the content
  spdlog::trace("Formatted tool tooltip for: {}", title);
}

/**
 * @brief Show a tooltip for a menu item
 */
void TooltipManager::showMenuItemTooltip(const std::string &actionId,
                                         const std::string &title,
                                         const std::string &description) {
  TooltipContent content;
  content.title = title;
  content.description = description;
  content.type = TooltipType::MenuItem;
  content.shortcut = getToolShortcut(actionId);

  // Note: Actual tooltip display is handled by the UI layer (ImGUI)
  spdlog::trace("Formatted menu item tooltip for: {}", title);
}

/**
 * @brief Show a tooltip for a property
 */
void TooltipManager::showPropertyTooltip(const std::string &propertyPath,
                                         const std::string &currentValue,
                                         const std::string &constraints) {
  TooltipContent content;
  content.title = propertyPath;
  content.description = "Value: " + currentValue;
  if (!constraints.empty()) {
    content.description += "\nConstraints: " + constraints;
  }
  content.type = TooltipType::Property;

  // Note: Actual tooltip display is handled by the UI layer (ImGUI)
  spdlog::trace("Formatted property tooltip for: {}", propertyPath);
}

/**
 * @brief Show a tooltip for a selected object
 */
void TooltipManager::showObjectTooltip(
    uint32_t objectId, const std::string &objectType,
    const std::unordered_map<std::string, std::string> &properties) {
  TooltipContent content;
  content.title = objectType + " #" + std::to_string(objectId);
  content.type = TooltipType::Object;

  std::ostringstream desc;
  desc << "Type: " << objectType;
  if (!properties.empty()) {
    desc << "\nProperties:";
    for (const auto &[name, value] : properties) {
      desc << "\n  " << name << ": " << value;
    }
  }
  content.description = desc.str();

  // Note: Actual tooltip display is handled by the UI layer (ImGUI)
  spdlog::trace("Formatted object tooltip for: {}", objectType);
}

/**
 * @brief Show a contextual help tooltip
 */
void TooltipManager::showHelpTooltip(const std::string &helpId,
                                     const std::string &title,
                                     const std::string &content) {
  TooltipContent tooltipContent;
  tooltipContent.title = title;
  tooltipContent.description = content;
  tooltipContent.type = TooltipType::ContextualHelp;

  // Note: Actual tooltip display is handled by the UI layer (ImGUI)
  spdlog::trace("Formatted help tooltip for: {}", title);
}

// ==========================================================================
// Query Methods (Stateless)
// ==========================================================================

/**
 * @brief Get tooltip content for a tool
 */
std::optional<TooltipContent>
TooltipManager::getToolTooltipContent(const std::string &toolId) const {
  TooltipContent content;
  content.title = toolId;
  content.type = TooltipType::Tool;
  content.shortcut = getToolShortcut(toolId);
  return content;
}

/**
 * @brief Get tooltip content for a menu item
 */
std::optional<TooltipContent>
TooltipManager::getMenuItemTooltipContent(const std::string &actionId) const {
  TooltipContent content;
  content.title = actionId;
  content.type = TooltipType::MenuItem;
  content.shortcut = getToolShortcut(actionId);
  return content;
}

/**
 * @brief Check if tooltips are enabled
 */
bool TooltipManager::areTooltipsEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.enabled;
}

/**
 * @brief Get the tooltip delay in milliseconds
 */
int TooltipManager::getTooltipDelay() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.delay;
}

/**
 * @brief Get the tooltip duration in milliseconds
 */
int TooltipManager::getTooltipDuration() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.duration;
}

/**
 * @brief Get the maximum tooltip width in pixels
 */
int TooltipManager::getTooltipMaxWidth() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.maxWidth;
}

/**
 * @brief Get the tooltip position mode
 */
TooltipPosition TooltipManager::getTooltipPosition() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.position;
}

/**
 * @brief Check if shortcuts should be shown in tooltips
 */
bool TooltipManager::showShortcutsInTooltips() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.showShortcuts;
}

/**
 * @brief Check if descriptions should be shown in tooltips
 */
bool TooltipManager::showDescriptionsInTooltips() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.showDescriptions;
}

/**
 * @brief Check if tooltip text should be wrapped
 */
bool TooltipManager::wrapTooltipText() const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();
  return cachedSettings_.wrapText;
}

// ==========================================================================
// Private Methods
// ==========================================================================

/**
 * @brief Update cached settings from UIFSMAdapter
 */
void TooltipManager::updateCachedSettings() const {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - cachedSettings_.lastUpdate)
                     .count();

  // Update if dirty or more than 100ms has passed
  if (cachedSettings_.dirty || elapsed > 100) {
    // Query UIFSMAdapter for tooltip settings
    auto tooltipSettings = fsmAdapter_.getTooltipSettings();

    // Update cached settings from UIFSMAdapter
    cachedSettings_.enabled = tooltipSettings.enabled;

    // Get config based on preset
    if (tooltipSettings.useCustom) {
      cachedSettings_.delay = tooltipSettings.customConfig.delay;
      cachedSettings_.duration = tooltipSettings.customConfig.duration;
      cachedSettings_.maxWidth = tooltipSettings.customConfig.maxWidth;
      cachedSettings_.showShortcuts =
          tooltipSettings.customConfig.showShortcuts;
      cachedSettings_.showDescriptions =
          tooltipSettings.customConfig.showDescriptions;
      cachedSettings_.wrapText = tooltipSettings.customConfig.wrapText;
    } else {
      // Use preset defaults
      switch (tooltipSettings.preset) {
      case TooltipPreset::Basic:
        cachedSettings_.delay = 500;
        cachedSettings_.duration = 0;
        cachedSettings_.maxWidth = 300;
        cachedSettings_.showShortcuts = false;
        cachedSettings_.showDescriptions = false;
        cachedSettings_.wrapText = false;
        break;
      case TooltipPreset::Detailed:
        cachedSettings_.delay = 500;
        cachedSettings_.duration = 0;
        cachedSettings_.maxWidth = 400;
        cachedSettings_.showShortcuts = true;
        cachedSettings_.showDescriptions = true;
        cachedSettings_.wrapText = true;
        break;
      case TooltipPreset::Minimal:
        cachedSettings_.delay = 300;
        cachedSettings_.duration = 0;
        cachedSettings_.maxWidth = 350;
        cachedSettings_.showShortcuts = true;
        cachedSettings_.showDescriptions = false;
        cachedSettings_.wrapText = true;
        break;
      default:
        break;
      }
    }

    cachedSettings_.lastUpdate = now;
    cachedSettings_.dirty = false;
  }
}

/**
 * @brief Mark cached settings as dirty
 */
void TooltipManager::markCacheDirty() {
  std::lock_guard<std::mutex> lock(mutex_);
  cachedSettings_.dirty = true;
}

/**
 * @brief Get the color for a tooltip type
 */
glm::vec4 TooltipManager::getTooltipColor(TooltipType type) const {
  // Get theme colors from UIFSMAdapter
  auto themeSettings = fsmAdapter_.getThemeSettings();

  // Use theme text color as base
  glm::vec4 baseColor = themeSettings.useCustom
                            ? themeSettings.customScheme.text
                            : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

  // Adjust color based on type
  switch (type) {
  case TooltipType::Tool:
    return baseColor;
  case TooltipType::MenuItem:
    return baseColor;
  case TooltipType::Property:
    return glm::vec4(0.8f, 0.9f, 1.0f, 1.0f); // Light blue tint
  case TooltipType::Object:
    return glm::vec4(0.9f, 0.85f, 1.0f, 1.0f); // Light purple tint
  case TooltipType::ContextualHelp:
    return glm::vec4(1.0f, 0.95f, 0.8f, 1.0f); // Light yellow tint
  default:
    return baseColor;
  }
}

/**
 * @brief Format tooltip content for display
 */
std::string
TooltipManager::formatTooltipContent(const TooltipContent &content) const {
  std::lock_guard<std::mutex> lock(mutex_);
  updateCachedSettings();

  std::ostringstream result;

  // Always show title
  result << content.title;

  // Show shortcut if enabled and available
  if (cachedSettings_.showShortcuts && !content.shortcut.empty()) {
    result << " [" << content.shortcut << "]";
  }

  // Show description if enabled and available
  if (cachedSettings_.showDescriptions && !content.description.empty()) {
    result << "\n" << content.description;
  }

  return result.str();
}

/**
 * @brief Get shortcut for a tool from ShortcutManager
 */
std::string TooltipManager::getToolShortcut(const std::string &toolId) const {
  // Query UIFSMAdapter for shortcut settings
  auto shortcutSettings = fsmAdapter_.getShortcutSettings();

  // Look for shortcut with matching action (exact match, not substring)
  for (const auto &[id, shortcut] : shortcutSettings.shortcuts) {
    if (shortcut.action == toolId) {
      // Convert KeyCombo to string representation
      const char *modifier = "";
      switch (shortcut.keyCombo.modifier) {
      case KeyModifier::Ctrl:
        modifier = "Ctrl+";
        break;
      case KeyModifier::Shift:
        modifier = "Shift+";
        break;
      case KeyModifier::Alt:
        modifier = "Alt+";
        break;
      case KeyModifier::CtrlShift:
        modifier = "Ctrl+Shift+";
        break;
      case KeyModifier::CtrlAlt:
        modifier = "Ctrl+Alt+";
        break;
      case KeyModifier::ShiftAlt:
        modifier = "Shift+Alt+";
        break;
      case KeyModifier::CtrlShiftAlt:
        modifier = "Ctrl+Shift+Alt+";
        break;
      default:
        break;
      }

      // Map key code to readable name
      std::string keyName = getKeyName(shortcut.keyCombo.keyCode);
      return std::string(modifier) + keyName;
    }
  }

  return "";
}

} // namespace view
