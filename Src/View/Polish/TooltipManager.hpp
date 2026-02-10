#pragma once

#include <View/UIFSMAdapter.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace view {

// Forward declaration
class UIFSMAdapter;

/**
 * @brief Tooltip position mode
 *
 * Defines how tooltips are positioned relative to the mouse cursor.
 */
enum class TooltipPosition {
  FollowMouse, ///< Tooltip follows the mouse cursor
  Fixed,       ///< Tooltip appears at a fixed position
  Smart        ///< Tooltip intelligently avoids screen edges
};

/**
 * @brief Tooltip type for categorization
 *
 * Defines the type of tooltip for color coding and content formatting.
 */
enum class TooltipType {
  Tool,          ///< Tool button tooltip (name + shortcut + description)
  MenuItem,      ///< Menu item tooltip (extended description)
  Property,      ///< Property tooltip (value details + constraints)
  Object,        ///< Object tooltip (type + properties + selection info)
  ContextualHelp ///< Contextual help tooltip
};

/**
 * @brief Tooltip content structure
 *
 * Contains all content elements for a tooltip including title,
 * description, shortcut key, icon, and color coding.
 */
struct TooltipContent {
  /// Tooltip title (displayed in bold)
  std::string title;

  /// Tooltip description (multi-line, can be empty)
  std::string description;

  /// Shortcut key (if applicable, empty string otherwise)
  std::string shortcut;

  /// Optional icon identifier (empty string for no icon)
  std::string icon;

  /// Tooltip type for color coding
  TooltipType type = TooltipType::Tool;

  /**
   * @brief Default constructor
   */
  TooltipContent() = default;

  /**
   * @brief Constructor with all fields
   */
  TooltipContent(std::string title, std::string description = "",
                 std::string shortcut = "", std::string icon = "",
                 TooltipType type = TooltipType::Tool)
      : title(std::move(title)), description(std::move(description)),
        shortcut(std::move(shortcut)), icon(std::move(icon)), type(type) {}

  /**
   * @brief Check if content is valid (has at least a title)
   * @return true if title is not empty
   */
  bool isValid() const { return !title.empty(); }
};

/**
 * @brief Manages UI tooltips for TFCADIR CAD application
 *
 * TooltipManager is a stateless coordinator that handles tooltip content
 * formatting and settings queries. All tooltip settings are stored in
 * UIFSMAdapter following the Phase 7 architecture pattern.
 *
 * This manager:
 * - Queries UIFSMAdapter for all tooltip settings (no local state storage)
 * - Formats tooltip content based on settings
 * - Provides thread-safe operations with mutex protection
 * - Supports multiple tooltip types with color coding
 *
 * Note: Hover state tracking and tooltip display are handled by the UI layer
 * (e.g., ImGUI widgets), not by this coordinator.
 */
class TooltipManager {
public:
  /**
   * @brief Construct a TooltipManager
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   */
  explicit TooltipManager(UIFSMAdapter &fsmAdapter);

  /**
   * @brief Destructor
   */
  ~TooltipManager();

  // ==========================================================================
  // Tooltip Display Methods
  // ==========================================================================

  /**
   * @brief Show a tooltip for a tool button
   * @param toolId The tool identifier
   * @param title The tool name
   * @param description The tool description
   *
   * Formats tooltip content for a tool button, including
   * the tool name, description, and associated keyboard shortcut.
   * Note: Actual tooltip display is handled by the UI layer (ImGUI).
   */
  void showToolTooltip(const std::string &toolId, const std::string &title,
                       const std::string &description);

  /**
   * @brief Show a tooltip for a menu item
   * @param actionId The action identifier
   * @param title The menu item title
   * @param description Extended description of the menu item
   *
   * Formats tooltip content for a menu item with extended
   * description content.
   * Note: Actual tooltip display is handled by the UI layer (ImGUI).
   */
  void showMenuItemTooltip(const std::string &actionId,
                           const std::string &title,
                           const std::string &description);

  /**
   * @brief Show a tooltip for a property
   * @param propertyPath The property path (e.g., "position.x")
   * @param currentValue The current value of the property
   * @param constraints Optional constraints description
   *
   * Formats tooltip content for a property showing the
   * current value and any applicable constraints.
   * Note: Actual tooltip display is handled by the UI layer (ImGUI).
   */
  void showPropertyTooltip(const std::string &propertyPath,
                           const std::string &currentValue,
                           const std::string &constraints = "");

  /**
   * @brief Show a tooltip for a selected object
   * @param objectId The object/figure ID
   * @param objectType The object type (e.g., "Line3D", "Circle3D")
   * @param properties Map of property names to values
   *
   * Formats tooltip content for a selected object showing
   * its type and properties.
   * Note: Actual tooltip display is handled by the UI layer (ImGUI).
   */
  void showObjectTooltip(
      uint32_t objectId, const std::string &objectType,
      const std::unordered_map<std::string, std::string> &properties);

  /**
   * @brief Show a contextual help tooltip
   * @param helpId The help identifier
   * @param title The help title
   * @param content The help content
   *
   * Formats tooltip content for contextual help with the
   * provided content.
   * Note: Actual tooltip display is handled by the UI layer (ImGUI).
   */
  void showHelpTooltip(const std::string &helpId, const std::string &title,
                       const std::string &content);

  // ==========================================================================
  // Query Methods (Stateless)
  // ==========================================================================

  /**
   * @brief Get tooltip content for a tool
   * @param toolId The tool identifier
   * @return Optional tooltip content if found
   *
   * Queries the ShortcutManager for the tool's shortcut and
   * constructs appropriate tooltip content.
   */
  std::optional<TooltipContent>
  getToolTooltipContent(const std::string &toolId) const;

  /**
   * @brief Get tooltip content for a menu item
   * @param actionId The action identifier
   * @return Optional tooltip content if found
   */
  std::optional<TooltipContent>
  getMenuItemTooltipContent(const std::string &actionId) const;

  /**
   * @brief Check if tooltips are enabled
   * @return true if tooltips are enabled
   *
   * Queries UIFSMAdapter for the current tooltip enabled setting.
   */
  bool areTooltipsEnabled() const;

  /**
   * @brief Get the tooltip delay in milliseconds
   * @return Delay before showing tooltip
   *
   * Queries UIFSMAdapter for the current tooltip delay setting.
   */
  int getTooltipDelay() const;

  /**
   * @brief Get the tooltip duration in milliseconds
   * @return Duration to show tooltip (0 = until mouse moves)
   *
   * Queries UIFSMAdapter for the current tooltip duration setting.
   */
  int getTooltipDuration() const;

  /**
   * @brief Get the maximum tooltip width in pixels
   * @return Maximum width for tooltip text wrapping
   *
   * Queries UIFSMAdapter for the current tooltip max width setting.
   */
  int getTooltipMaxWidth() const;

  /**
   * @brief Get the tooltip position mode
   * @return Current tooltip position mode
   *
   * Queries UIFSMAdapter for the current tooltip position setting.
   */
  TooltipPosition getTooltipPosition() const;

  /**
   * @brief Check if shortcuts should be shown in tooltips
   * @return true if shortcuts are shown in tooltips
   *
   * Queries UIFSMAdapter for the current show shortcuts setting.
   */
  bool showShortcutsInTooltips() const;

  /**
   * @brief Check if descriptions should be shown in tooltips
   * @return true if descriptions are shown in tooltips
   *
   * Queries UIFSMAdapter for the current show descriptions setting.
   */
  bool showDescriptionsInTooltips() const;

  /**
   * @brief Check if tooltip text should be wrapped
   * @return true if text wrapping is enabled
   *
   * Queries UIFSMAdapter for the current wrap text setting.
   */
  bool wrapTooltipText() const;

private:
  /// Reference to the UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Mutex for thread-safe access to cached settings
  mutable std::mutex mutex_;

  // ==========================================================================
  // Cached Settings (with dirty flag for 100ms rate limiting)
  // ==========================================================================

  /// Cached tooltip settings
  struct CachedSettings {
    bool enabled = true;
    int delay = 500;
    int duration = 0;
    int maxWidth = 400;
    TooltipPosition position = TooltipPosition::FollowMouse;
    bool showShortcuts = true;
    bool showDescriptions = true;
    bool wrapText = true;

    /// Timestamp when cache was last updated
    std::chrono::steady_clock::time_point lastUpdate;

    /// Whether cache is dirty and needs refresh
    bool dirty = true;
  };

  /// Thread-local cached settings for efficient access
  mutable CachedSettings cachedSettings_;

  /**
   * @brief Update cached settings from UIFSMAdapter
   *
   * Refreshes the cached settings from UIFSMAdapter if the cache
   * is dirty or more than 100ms has passed since last update.
   */
  void updateCachedSettings();

  /**
   * @brief Mark cached settings as dirty
   *
   * Forces a refresh of cached settings on next access.
   */
  void markCacheDirty();

  /**
   * @brief Get the color for a tooltip type
   * @param type The tooltip type
   * @return Color for the tooltip (RGBA)
   *
   * Returns the appropriate color for the tooltip based on
   * the current theme settings.
   */
  glm::vec4 getTooltipColor(TooltipType type) const;

  /**
   * @brief Format tooltip content for display
   * @param content The tooltip content
   * @return Formatted text string
   *
   * Formats the tooltip content based on current settings
   * (show/hide shortcuts, descriptions, text wrapping).
   */
  std::string formatTooltipContent(const TooltipContent &content) const;

  /**
   * @brief Get shortcut for a tool from ShortcutManager
   * @param toolId The tool identifier
   * @return Shortcut string or empty string if not found
   *
   * Queries the UIFSMAdapter for shortcut information.
   */
  std::string getToolShortcut(const std::string &toolId) const;
};

} // namespace view
