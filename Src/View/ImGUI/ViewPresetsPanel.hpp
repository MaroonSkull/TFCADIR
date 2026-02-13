#pragma once

#include <View/Navigation/NavigationTypes.hpp>
#include <chrono>
#include <string>

namespace view {

// Forward declaration
class NavigationManager;

/**
 * @brief ImGUI panel for view preset selection
 * @details Provides UI buttons for selecting predefined view presets (Top,
 * Front, Right, Isometric, etc.). Implements caching with dirty flag and 100ms
 *          update rate limiting for performance (max 10 Hz).
 *
 *          This panel follows the query-based pattern from ToolOptionsPanel:
 *          - Queries NavigationManager for current view preset
 *          - Queries NavigationManager for available view presets
 *          - No direct state storage - all state queried from NavigationManager
 *          - Uses dirty flag to track when UI needs to be rebuilt
 *          - Uses rate limiting to prevent excessive UI updates
 */
class ViewPresetsPanel {
public:
  /**
   * @brief Constructor for ViewPresetsPanel
   * @param navigationManager Reference to NavigationManager for queries
   */
  explicit ViewPresetsPanel(NavigationManager &navigationManager);

  /**
   * @brief Renders the view presets panel
   * @details Renders ImGUI buttons for each view preset. Uses rate limiting
   *          to prevent excessive updates (max 10 Hz). Updates dirty flag
   *          when view preset changes.
   */
  void render();

  /**
   * @brief Marks the panel as needing a rebuild
   * @details Sets the dirty flag, forcing the next render() call to rebuild
   *          the UI. Call this when the view preset changes externally.
   */
  void markDirty();

  /**
   * @brief Checks if the panel is dirty (needs rebuilding)
   * @return true if the panel needs to be rebuilt
   */
  [[nodiscard]] bool isDirty() const;

  /**
   * @brief Clears the dirty flag
   * @details Call this after rebuilding the UI to indicate that the panel
   *          is up to date.
   */
  void clearDirty();

private:
  /// Reference to NavigationManager for queries
  NavigationManager &navigationManager_;

  /// Flag to track if UI needs to be rebuilt
  bool isDirty_;

  /// Time of last UI update for rate limiting
  std::chrono::steady_clock::time_point lastUpdateTime_;

  /// Minimum time between UI updates (100ms = max 10 Hz)
  static constexpr std::chrono::milliseconds MIN_UPDATE_INTERVAL{100};

  /**
   * @brief Renders a single view preset button
   * @param preset The view preset to render
   * @param label The label for the button
   * @return true if the button was clicked
   */
  bool renderPresetButton(ViewPreset preset, const char *label);

  /**
   * @brief Checks if enough time has passed since the last update
   * @return true if at least MIN_UPDATE_INTERVAL has passed
   */
  [[nodiscard]] bool shouldUpdate() const;

  /**
   * @brief Gets the display label for a view preset
   * @param preset The view preset
   * @return The display label for the preset
   */
  [[nodiscard]] static const char *getPresetLabel(ViewPreset preset);
};

} // namespace view
