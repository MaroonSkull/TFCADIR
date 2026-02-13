#pragma once

#include <View/Navigation/NavigationTypes.hpp>
#include <functional>
#include <memory>

namespace view {

// Forward declaration
class ViewPresetManager;

/**
 * @brief ImGUI widget for view preset buttons
 * @details Provides a 3x2 button grid for quick view preset selection.
 *          Layout: Top/Front/Right on row 1, Bottom/Back/Left on row 2,
 *          ISO centered on row 3.
 *
 *          Button size: 60x40 pixels
 *          Features:
 *          - Active state highlighting
 *          - Hover tooltips with preset name and shortcut
 */
class ViewPresetButtons {
public:
  /**
   * @brief Constructor for ViewPresetButtons
   * @param presetManager Shared pointer to ViewPresetManager for preset
   * operations
   */
  explicit ViewPresetButtons(std::shared_ptr<ViewPresetManager> presetManager);

  /**
   * @brief Destructor
   */
  ~ViewPresetButtons() = default;

  /**
   * @brief Render the view preset buttons
   * @details Renders a 3x2 button grid with ISO centered below.
   *          Highlights the active preset and shows tooltips on hover.
   */
  void render();

  /**
   * @brief Set callback for preset button click
   * @param callback Function to call when a preset button is clicked
   */
  void setOnPresetClicked(std::function<void(ViewPreset)> callback);

private:
  /// Shared pointer to ViewPresetManager
  std::shared_ptr<ViewPresetManager> presetManager_;

  /// Callback for preset button click
  std::function<void(ViewPreset)> onPresetClicked_;

  /// Button dimensions
  static constexpr float BUTTON_WIDTH = 60.0f;
  static constexpr float BUTTON_HEIGHT = 40.0f;
  static constexpr float BUTTON_SPACING = 4.0f;

  /**
   * @brief Render a single preset button
   * @param preset The view preset for this button
   * @param label The display label for the button
   * @return true if the button was clicked
   */
  bool renderPresetButton(ViewPreset preset, const char *label);

  /**
   * @brief Check if a preset is currently active
   * @param preset The preset to check
   * @return true if the preset is active
   */
  [[nodiscard]] bool isPresetActive(ViewPreset preset) const;
};

} // namespace view
