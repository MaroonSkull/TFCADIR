/**
 * @file SettingsConfig.hpp
 * @brief Configuration structures for Grid and Display settings
 *
 * This file contains the configuration data structures used by the
 * GridSettings and DisplaySettings dialogs, as well as the SettingsManager
 * for persistence.
 */

#pragma once

#include <glm/glm.hpp>
#include <string>

namespace view {
namespace settings {

/**
 * @brief Grid type enumeration for different grid visualization modes
 */
enum class GridType {
  Cartesian, ///< Standard X-Y grid with perpendicular lines
  Isometric, ///< 30-degree isometric grid for 3D sketching
  Polar      ///< Radial grid with concentric circles and radial lines
};

/**
 * @brief Configuration for grid display settings
 *
 * Contains all configurable parameters for the grid visualization
 * including visibility, spacing, colors, and grid type.
 */
struct GridConfig {
  /// Whether the grid is visible
  bool showGrid = true;

  /// Whether minor grid lines are displayed
  bool showMinorGridLines = false;

  /// Spacing between major grid lines in world units
  float majorGridSpacing = 10.0f;

  /// Number of minor divisions between major lines
  int minorDivisions = 10;

  /// Color of major grid lines (RGBA)
  glm::vec4 majorGridColor = glm::vec4(0.26f, 0.26f, 0.26f, 1.0f);

  /// Color of minor grid lines (RGBA)
  glm::vec4 minorGridColor = glm::vec4(0.20f, 0.20f, 0.20f, 1.0f);

  /// Opacity of the grid (0.0 = transparent, 1.0 = opaque)
  float gridOpacity = 0.8f;

  /// Type of grid visualization
  GridType gridType = GridType::Cartesian;

  /**
   * @brief Equality comparison operator
   * @param other The GridConfig to compare with
   * @return true if all settings are equal
   */
  bool operator==(const GridConfig &other) const;

  /**
   * @brief Inequality comparison operator
   * @param other The GridConfig to compare with
   * @return true if any settings differ
   */
  bool operator!=(const GridConfig &other) const;
};

/**
 * @brief Theme enumeration for UI appearance
 */
enum class Theme {
  Dark,  ///< Dark theme with light text on dark background
  Light, ///< Light theme with dark text on light background
  Custom ///< User-customized theme
};

/**
 * @brief UI size enumeration for interface scaling
 */
enum class UISize {
  Small,  ///< Compact UI elements
  Medium, ///< Standard UI element size
  Large   ///< Larger UI elements for accessibility
};

/**
 * @brief Configuration for display and appearance settings
 *
 * Contains all configurable parameters for the application's visual
 * appearance including theme, colors, fonts, and UI scaling.
 */
struct DisplayConfig {
  /// Current UI theme
  Theme theme = Theme::Dark;

  /// Background color of the canvas (RGBA)
  glm::vec4 backgroundColor = glm::vec4(0.12f, 0.12f, 0.12f, 1.0f);

  /// Foreground/text color (RGBA)
  glm::vec4 foregroundColor = glm::vec4(0.86f, 0.86f, 0.86f, 1.0f);

  /// Accent color for highlights and selections (RGBA)
  glm::vec4 accentColor = glm::vec4(0.0f, 0.48f, 0.68f, 1.0f);

  /// Selection highlight color (RGBA)
  glm::vec4 selectionColor = glm::vec4(1.0f, 0.65f, 0.0f, 1.0f);

  /// Color of the X-axis (RGBA)
  glm::vec4 axisXColor = glm::vec4(1.0f, 0.42f, 0.42f, 1.0f);

  /// Color of the Y-axis (RGBA)
  glm::vec4 axisYColor = glm::vec4(0.31f, 0.80f, 0.77f, 1.0f);

  /// Color of the Z-axis (RGBA)
  glm::vec4 axisZColor = glm::vec4(1.0f, 0.90f, 0.43f, 1.0f);

  /// Name of the current font
  std::string fontName = "Default";

  /// Font size in points
  int fontSize = 14;

  /// UI element size scaling
  UISize uiSize = UISize::Medium;

  /**
   * @brief Equality comparison operator
   * @param other The DisplayConfig to compare with
   * @return true if all settings are equal
   */
  bool operator==(const DisplayConfig &other) const;

  /**
   * @brief Inequality comparison operator
   * @param other The DisplayConfig to compare with
   * @return true if any settings differ
   */
  bool operator!=(const DisplayConfig &other) const;
};

/**
 * @brief Combined application settings structure
 *
 * Contains all application settings including grid and display configurations.
 * Used by SettingsManager for loading and saving settings.
 */
struct AppSettings {
  /// Grid display configuration
  GridConfig gridConfig;

  /// Display and appearance configuration
  DisplayConfig displayConfig;

  /**
   * @brief Equality comparison operator
   * @param other The AppSettings to compare with
   * @return true if all settings are equal
   */
  bool operator==(const AppSettings &other) const;

  /**
   * @brief Inequality comparison operator
   * @param other The AppSettings to compare with
   * @return true if any settings differ
   */
  bool operator!=(const AppSettings &other) const;
};

/**
 * @brief Convert GridType to string representation
 * @param type The grid type to convert
 * @return String representation of the grid type
 */
std::string gridTypeToString(GridType type);

/**
 * @brief Convert string to GridType
 * @param str The string to convert
 * @return The corresponding GridType
 * @throws std::invalid_argument if string is not a valid grid type
 */
GridType stringToGridType(const std::string &str);

/**
 * @brief Convert Theme to string representation
 * @param theme The theme to convert
 * @return String representation of the theme
 */
std::string themeToString(Theme theme);

/**
 * @brief Convert string to Theme
 * @param str The string to convert
 * @return The corresponding Theme
 * @throws std::invalid_argument if string is not a valid theme
 */
Theme stringToTheme(const std::string &str);

/**
 * @brief Convert UISize to string representation
 * @param size The UI size to convert
 * @return String representation of the UI size
 */
std::string uiSizeToString(UISize size);

/**
 * @brief Convert string to UISize
 * @param str The string to convert
 * @return The corresponding UISize
 * @throws std::invalid_argument if string is not a valid UI size
 */
UISize stringToUISize(const std::string &str);

} // namespace settings
} // namespace view
