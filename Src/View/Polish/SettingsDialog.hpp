#ifndef TFCADIR_VIEW_POLISH_SETTINGSDIALOG_HPP
#define TFCADIR_VIEW_POLISH_SETTINGSDIALOG_HPP

#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

namespace view {

// Forward declarations
class UIFSMAdapter;
class ShortcutManager;
class ThemeManager;

/**
 * @brief Settings category enumeration
 *
 * Defines the available settings categories in the settings dialog.
 */
enum class SettingsCategory {
  General,     ///< General application settings
  Interface,   ///< User interface settings
  Input,       ///< Input device settings
  View,        ///< View and display settings
  Performance, ///< Performance settings
  Shortcuts,   ///< Keyboard shortcuts
  Themes       ///< Theme customization
};

/**
 * @brief General application settings
 *
 * Contains configuration for general application behavior.
 */
struct GeneralSettings {
  /// Default mode (2D or 3D)
  std::string defaultMode = "2D";

  /// Whether to show welcome screen on startup
  bool showWelcomeScreen = true;

  /// Auto-save interval in minutes (0 = disabled)
  int autoSaveInterval = 5;

  /// Number of backup files to keep
  int backupCount = 3;

  /// Units for measurements ("mm", "inches")
  std::string units = "mm";

  /// Whether to check for updates automatically
  bool checkUpdates = true;
};

/**
 * @brief User interface settings
 *
 * Contains configuration for the user interface appearance and behavior.
 */
struct InterfaceSettings {
  /// Current theme name
  std::string theme = "Dark";

  /// Icon size ("Small", "Medium", "Large")
  std::string iconSize = "Medium";

  /// Font name
  std::string font = "Default";

  /// Whether to show tooltips
  bool showTooltips = true;

  /// Tooltip delay in milliseconds
  int tooltipDelay = 500;

  /// Whether to remember window position
  bool rememberWindow = true;

  /// Whether to remember panel layout
  bool rememberLayout = true;

  /// Panel width in pixels
  int panelWidth = 280;

  /// Names of startup panels
  std::vector<std::string> startupPanels = {"Tools", "Outliner", "Properties"};
};

/**
 * @brief Input device settings
 *
 * Contains configuration for mouse, keyboard, and tablet input.
 */
struct InputSettings {
  /// Orbit sensitivity (0.0 to 1.0)
  float orbitSensitivity = 0.5f;

  /// Zoom sensitivity (0.0 to 1.0)
  float zoomSensitivity = 0.6f;

  /// Pan speed (0.0 to 1.0)
  float panSpeed = 0.5f;

  /// Whether scroll wheel zooms to cursor position
  bool scrollZoomToCursor = true;

  /// Action for double-click ("FitToView", "Edit", "None")
  std::string doubleClickAction = "FitToView";

  /// Whether tablet/stylus support is enabled
  bool tabletSupport = false;

  /// Pressure sensitivity (0.0 to 1.0)
  float pressureSensitivity = 0.7f;

  /// Input repeat delay in milliseconds
  int inputRepeatDelay = 500;

  /// Input repeat rate per second
  int inputRepeatRate = 20;
};

/**
 * @brief View and display settings
 *
 * Contains configuration for grid, axes, and visual elements.
 */
struct ViewSettings {
  /// Whether to show grid
  bool showGrid = true;

  /// Whether to show minor grid lines
  bool showMinorGrid = false;

  /// Whether to show axes
  bool showAxis = true;

  /// Whether to show origin
  bool showOrigin = false;

  /// Grid spacing for major lines
  float gridSpacingMajor = 10.0f;

  /// Number of minor divisions
  int gridSpacingMinor = 10;

  /// Grid color (hex string)
  std::string gridColor = "#444444";

  /// Grid opacity (0.0 to 1.0)
  float gridOpacity = 0.8f;
};

/**
 * @brief Performance settings
 *
 * Contains configuration for rendering and performance options.
 */
struct PerformanceSettings {
  /// Whether VSync is enabled
  bool vsync = true;

  /// Maximum FPS (0 = unlimited)
  int maxFPS = 60;

  /// Anti-aliasing level (0, 2, 4, 8)
  int antiAliasing = 4;

  /// Anisotropic filtering level (0, 2, 4, 8, 16)
  int anisotropicFiltering = 16;

  /// Render distance
  int renderDistance = 1000;

  /// Shadow quality ("Low", "Medium", "High", "Ultra")
  std::string shadowQuality = "Medium";

  /// Effects quality ("Low", "Medium", "High", "Ultra")
  std::string effectsQuality = "High";
};

/**
 * @brief Shortcut entry for settings dialog
 *
 * Represents a keyboard shortcut in the shortcuts tab.
 */
struct ShortcutEntry {
  /// Action identifier
  std::string action;

  /// Key combination string (e.g., "Ctrl+Z")
  std::string shortcut;

  /// Human-readable description
  std::string description;
};

/**
 * @brief Theme color scheme
 *
 * Contains color definitions for a theme.
 */
struct ThemeColors {
  /// Background color (hex string)
  std::string background = "#1E1E1E";

  /// Foreground/text color (hex string)
  std::string foreground = "#D4D4D4";

  /// Accent color (hex string)
  std::string accent = "#007ACC";

  /// Border color (hex string)
  std::string border = "#3E3E42";

  /// Header color (hex string)
  std::string header = "#2D2D30";

  /// Selection color (hex string)
  std::string selection = "#264F78";
};

/**
 * @brief Theme settings
 *
 * Contains configuration for theme customization.
 */
struct ThemeSettings {
  /// Current theme name
  std::string currentTheme = "Dark";

  /// Custom color scheme
  ThemeColors customColors;

  /// Whether this is a custom theme
  bool isCustom = false;
};

/**
 * @brief Complete settings structure
 *
 * Aggregates all settings categories into a single structure.
 */
struct Settings {
  GeneralSettings general;
  InterfaceSettings interface;
  InputSettings input;
  ViewSettings view;
  PerformanceSettings performance;
  std::vector<ShortcutEntry> shortcuts;
  ThemeSettings theme;
};

/**
 * @brief Settings dialog for TFCADIR application
 *
 * SettingsDialog is a modal ImGUI dialog that provides a tabbed interface
 * for managing application settings. It follows the stateless coordinator
 * pattern, querying UIFSMAdapter for all settings values and only storing
 * transient UI state.
 *
 * This dialog:
 * - Renders a tabbed interface with 7 settings categories
 * - Applies settings immediately with live preview
 * - Saves/loads settings from YAML file
 * - Integrates with ShortcutManager and ThemeManager
 *
 * No local domain state is stored - all settings come from UIFSMAdapter.
 */
class SettingsDialog {
public:
  /**
   * @brief Construct a SettingsDialog
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   * @param shortcutManager Reference to the ShortcutManager
   * @param themeManager Reference to the ThemeManager (may be null)
   */
  SettingsDialog(UIFSMAdapter &fsmAdapter, ShortcutManager &shortcutManager,
                 ThemeManager *themeManager = nullptr);

  /**
   * @brief Destructor
   */
  ~SettingsDialog();

  // ==========================================================================
  // Dialog Control
  // ==========================================================================

  /**
   * @brief Show the settings dialog
   */
  void show();

  /**
   * @brief Hide the settings dialog
   */
  void hide();

  /**
   * @brief Check if the dialog is currently visible
   * @return true if the dialog is visible
   */
  bool isVisible() const;

  /**
   * @brief Render the settings dialog
   *
   * Renders the modal dialog with tabbed interface.
   * Should be called each frame when the dialog is visible.
   */
  void render();

  // ==========================================================================
  // Settings Operations
  // ==========================================================================

  /**
   * @brief Apply all pending settings changes
   *
   * Applies all modified settings to UIFSMAdapter and triggers
   * appropriate FSM events for cache invalidation.
   */
  void apply();

  /**
   * @brief Reset all settings to saved values
   *
   * Discards all pending changes and reloads settings from UIFSMAdapter.
   */
  void reset();

  /**
   * @brief Save all settings to file
   * @param filePath Path to the settings file (default: "settings.yaml")
   * @return true if save was successful
   *
   * Saves all current settings to a YAML file for persistence.
   */
  bool saveToFile(const std::string &filePath = "settings.yaml");

  /**
   * @brief Load all settings from file
   * @param filePath Path to the settings file (default: "settings.yaml")
   * @return true if load was successful
   *
   * Loads settings from a YAML file and applies them to UIFSMAdapter.
   */
  bool loadFromFile(const std::string &filePath = "settings.yaml");

private:
  /// Reference to the UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Reference to the ShortcutManager (no ownership)
  ShortcutManager &shortcutManager_;

  /// Pointer to the ThemeManager (may be null, no ownership)
  ThemeManager *themeManager_;

  // UI-only state (transient)
  /// Whether the dialog is currently visible
  bool visible_ = false;

  /// Currently selected tab
  SettingsCategory currentTab_ = SettingsCategory::General;

  /// Working copy of settings (for editing before apply)
  Settings workingSettings_;

  /// Original settings (for reset functionality)
  Settings originalSettings_;

  /// Whether settings have been modified
  bool settingsModified_ = false;

  /// Search filter text for shortcuts tab
  char shortcutFilter_[256] = "";

  /// Currently selected shortcut for editing
  int selectedShortcutIndex_ = -1;

  /// Whether we are capturing a new shortcut
  bool capturingShortcut_ = false;

  /// Selected theme preset
  int selectedThemePreset_ = 0;

  // ==========================================================================
  // Tab Rendering Methods
  // ==========================================================================

  /**
   * @brief Render the General settings tab
   */
  void renderGeneralTab();

  /**
   * @brief Render the Interface settings tab
   */
  void renderInterfaceTab();

  /**
   * @brief Render the Input settings tab
   */
  void renderInputTab();

  /**
   * @brief Render the View settings tab
   */
  void renderViewTab();

  /**
   * @brief Render the Performance settings tab
   */
  void renderPerformanceTab();

  /**
   * @brief Render the Shortcuts settings tab
   */
  void renderShortcutsTab();

  /**
   * @brief Render the Themes settings tab
   */
  void renderThemesTab();

  // ==========================================================================
  // Helper Methods
  // ==========================================================================

  /**
   * @brief Load current settings from UIFSMAdapter
   *
   * Queries UIFSMAdapter for all current settings and populates
   * the workingSettings_ structure.
   */
  void loadSettingsFromAdapter();

  /**
   * @brief Apply settings to UIFSMAdapter
   *
   * Applies all settings from workingSettings_ to UIFSMAdapter
   * and triggers appropriate FSM events.
   */
  void applySettingsToAdapter();

  /**
   * @brief Mark settings as modified
   */
  void markModified();

  /**
   * @brief Convert hex color string to ImVec4
   * @param hex Hex color string (e.g., "#1E1E1E")
   * @return Color as ImVec4
   */
  ImVec4 hexToColor(const std::string &hex) const;

  /**
   * @brief Convert ImVec4 to hex color string
   * @param color Color as ImVec4
   * @return Hex color string
   */
  std::string colorToHex(const ImVec4 &color) const;

  /**
   * @brief Render a color picker for a theme color
   * @param label Label for the color picker
   * @param colorHex Reference to hex color string
   */
  void renderColorPicker(const char *label, std::string &colorHex);

  /**
   * @brief Filter shortcuts based on search text
   * @return Vector of filtered shortcut entries
   */
  std::vector<ShortcutEntry> getFilteredShortcuts() const;

  /**
   * @brief Get theme preset names
   * @return Vector of theme preset names
   */
  std::vector<std::string> getThemePresets() const;

  /**
   * @brief Apply a theme preset
   * @param presetName Name of the preset to apply
   */
  void applyThemePreset(const std::string &presetName);
};

} // namespace view

#endif // TFCADIR_VIEW_POLISH_SETTINGSDIALOG_HPP
