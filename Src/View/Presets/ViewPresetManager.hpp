#pragma once

#include <View/Navigation/NavigationTypes.hpp>
#include <glm/glm.hpp>
#include <string>

namespace view {

// Forward declaration
class NavigationManager;

/**
 * @brief Manager for view preset operations with keyboard shortcuts support
 * @details Provides a convenient API for view preset operations including
 *          camera transforms, display names, and keyboard shortcuts.
 *          Wraps NavigationManager for Phase 9.4 View Presets functionality.
 */
class ViewPresetManager {
public:
  /**
   * @brief Construct a ViewPresetManager
   * @param navigationManager Reference to NavigationManager for view operations
   */
  explicit ViewPresetManager(NavigationManager &navigationManager);

  /**
   * @brief Destructor
   */
  ~ViewPresetManager() = default;

  // ==========================================================================
  // View Preset Operations
  // ==========================================================================

  /**
   * @brief Apply a view preset with smooth camera transition
   * @param preset The view preset to apply
   * @param duration Transition duration in seconds (default 300ms)
   */
  void applyPreset(ViewPreset preset, float duration = 0.3f);

  /**
   * @brief Get the display name for a view preset
   * @param preset The view preset
   * @return Human-readable name for the preset
   */
  [[nodiscard]] static std::string getPresetName(ViewPreset preset);

  /**
   * @brief Get the keyboard shortcut for a view preset
   * @param preset The view preset
   * @return Keyboard shortcut string (e.g., "NumPad 7", "Ctrl+NumPad 7")
   */
  [[nodiscard]] static std::string getPresetShortcut(ViewPreset preset);

  /**
   * @brief Get the current view preset
   * @return The currently active view preset
   */
  [[nodiscard]] ViewPreset getCurrentPreset() const;

  /**
   * @brief Get camera state for a view preset
   * @param preset The view preset
   * @return Camera state (position, target, up vectors)
   */
  [[nodiscard]] static glm::vec3 getPresetCameraPosition(ViewPreset preset);

  /**
   * @brief Get camera target for a view preset
   * @param preset The view preset
   * @return Camera target vector
   */
  [[nodiscard]] static glm::vec3 getPresetCameraTarget(ViewPreset preset);

  /**
   * @brief Get camera up vector for a view preset
   * @param preset The view preset
   * @return Camera up vector
   */
  [[nodiscard]] static glm::vec3 getPresetCameraUp(ViewPreset preset);

private:
  /// Reference to NavigationManager for view operations
  NavigationManager &navigationManager_;
};

} // namespace view
