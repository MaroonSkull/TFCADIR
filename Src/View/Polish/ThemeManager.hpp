#pragma once

#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace view {

// Forward declaration
class UIFSMAdapter;

// ThemePreset and ColorScheme are now defined in UIFSMAdapter.hpp
// to follow the Phase 7 architecture pattern where all domain state
// is stored in UIFSMAdapter.

/**
 * @brief Manages ImGUI themes and color schemes
 *
 * ThemeManager is a stateless coordinator that manages ImGUI themes with
 * predefined presets, custom theme support, and YAML persistence.
 * All theme settings are stored in UIFSMAdapter following the Phase 7
 * architecture pattern.
 *
 * Features:
 * - Three predefined themes (Dark, Light, High Contrast)
 * - Custom theme support with user-defined colors
 * - YAML persistence for saving/loading themes
 * - ImGUI style application
 * - Color scheme management and editing
 */
class ThemeManager {
public:
  /**
   * @brief Construct a ThemeManager
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   */
  explicit ThemeManager(UIFSMAdapter &fsmAdapter);

  /**
   * @brief Destructor
   */
  ~ThemeManager();

  // ==========================================================================
  // Theme Management
  // ==========================================================================

  /**
   * @brief Set the current theme to a predefined preset
   * @param preset The theme preset to apply
   *
   * Applies the specified predefined theme and updates the current
   * color scheme. The theme change is stored in UIFSMAdapter.
   */
  void setTheme(ThemePreset preset);

  /**
   * @brief Set a custom theme with user-defined colors
   * @param scheme The custom color scheme to apply
   *
   * Applies the custom color scheme and sets the current theme
   * to ThemePreset::Custom. The scheme is stored in UIFSMAdapter.
   */
  void setCustomTheme(const ColorScheme &scheme);

  /**
   * @brief Get the current theme preset
   * @return Current theme preset
   */
  ThemePreset getCurrentTheme() const;

  /**
   * @brief Get the current color scheme
   * @return Const reference to the current color scheme
   */
  const ColorScheme &getCurrentColorScheme() const;

  // ==========================================================================
  // Color Access
  // ==========================================================================

  /**
   * @brief Get a color by name
   * @param colorName The name of the color (e.g., "windowBg", "accent")
   * @return The color value, or white if not found
   */
  glm::vec4 getColor(const std::string &colorName) const;

  /**
   * @brief Set a color by name
   * @param colorName The name of the color to set
   * @param color The new color value
   *
   * Updates the specified color in the current scheme and stores
   * the updated scheme in UIFSMAdapter. Automatically switches to
   * Custom theme if not already.
   */
  void setColor(const std::string &colorName, const glm::vec4 &color);

  // ==========================================================================
  // Persistence
  // ==========================================================================

  /**
   * @brief Save current theme to a YAML file
   * @param filepath Path to the output YAML file
   * @return true if save was successful, false otherwise
   *
   * Exports the current theme (preset and custom colors) to a YAML
   * configuration file for backup or sharing.
   */
  bool saveToFile(const std::string &filepath);

  /**
   * @brief Load theme from a YAML file
   * @param filepath Path to the input YAML file
   * @return true if load was successful, false otherwise
   *
   * Imports theme settings from a YAML configuration file. If the
   * file cannot be parsed or is invalid, returns false and does not
   * modify the current theme.
   */
  bool loadFromFile(const std::string &filepath);

  // ==========================================================================
  // ImGUI Integration
  // ==========================================================================

  /**
   * @brief Apply the current theme to an ImGUI style
   * @param style Reference to the ImGUI style to apply colors to
   *
   * Applies all colors from the current color scheme to the provided
   * ImGUI style structure. Call this after ImGui::StyleColorsDark or
   * similar to override with custom colors.
   */
  void applyTheme(ImGuiStyle &style) const;

  /**
   * @brief Render a theme editor UI
   *
   * Renders an ImGUI interface for editing theme colors. Allows users
   * to customize individual colors and switch between presets.
   * Call this within an ImGUI window to display the theme editor.
   */
  void renderThemeEditor();

  // ==========================================================================
  // Preset Access
  // ==========================================================================

  /**
   * @brief Get a predefined color scheme
   * @param preset The theme preset to retrieve
   * @return The color scheme for the specified preset
   */
  ColorScheme getPresetScheme(ThemePreset preset) const;

private:
  /// Reference to the UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Mutex for thread-safe access to internal state
  mutable std::mutex mutex_;

  /// Map from color name to color pointer for O(1) lookup
  std::map<std::string, glm::vec4 ColorScheme::*> colorMap_;

  /**
   * @brief Initialize the color name to member pointer map
   *
   * Builds the colorMap_ that maps color names to their corresponding
   * ColorScheme member pointers for fast lookup.
   */
  void buildColorMap();

  /**
   * @brief Create the dark theme color scheme
   * @return ColorScheme configured for dark theme
   *
   * Creates a professional dark theme suitable for CAD applications
   * with good contrast and reduced eye strain.
   */
  ColorScheme createDarkTheme() const;

  /**
   * @brief Create the light theme color scheme
   * @return ColorScheme configured for light theme
   *
   * Creates a light theme suitable for bright environments with
   * clear visibility.
   */
  ColorScheme createLightTheme() const;

  /**
   * @brief Create the high contrast theme color scheme
   * @return ColorScheme configured for high contrast theme
   *
   * Creates a high contrast theme for accessibility with maximum
   * visibility and clear color differentiation.
   */
  ColorScheme createHighContrastTheme() const;

  /**
   * @brief Convert hex color to glm::vec4
   * @param hex Hex color value (0xRRGGBB or 0xAARRGGBB)
   * @return Color as RGBA vector (0.0 to 1.0 range)
   */
  glm::vec4 hexToVec4(uint32_t hex) const;

  /**
   * @brief Convert glm::vec4 to hex color
   * @param color Color as RGBA vector (0.0 to 1.0 range)
   * @return Hex color value (0xAARRGGBB format)
   */
  uint32_t vec4ToHex(const glm::vec4 &color) const;

  /**
   * @brief Convert hex string to glm::vec4
   * @param hexString Hex color string (e.g., "#1E1E1E" or "1E1E1E")
   * @return Color as RGBA vector (0.0 to 1.0 range)
   */
  glm::vec4 hexStringToVec4(const std::string &hexString) const;

  /**
   * @brief Convert glm::vec4 to hex string
   * @param color Color as RGBA vector (0.0 to 1.0 range)
   * @return Hex color string (e.g., "#1E1E1E")
   */
  std::string vec4ToHexString(const glm::vec4 &color) const;

  /**
   * @brief Get theme preset name as string
   * @param preset The theme preset
   * @return String representation of the preset
   */
  std::string presetToString(ThemePreset preset) const;

  /**
   * @brief Parse theme preset from string
   * @param presetString String representation of the preset
   * @return ThemePreset value, or Dark if not recognized
   */
  ThemePreset stringToPreset(const std::string &presetString) const;
};

} // namespace view
