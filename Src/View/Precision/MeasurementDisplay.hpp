#pragma once

#include <View/Precision/MeasurementManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <optional>

namespace view {

// ==========================================================================
// Phase 6: Precision & Snapping - MeasurementDisplay
// ==========================================================================

/**
 * @brief Data structure for measurement display
 *
 * Contains all measurement values and coordinate information
 * for display in the measurement overlay.
 */
struct MeasurementData {
  /// Distance measurement (if applicable)
  std::optional<float> distance;

  /// Angle measurement in degrees (if applicable)
  std::optional<float> angle;

  /// Area measurement (if applicable)
  std::optional<float> area;

  /// Perimeter measurement (if applicable)
  std::optional<float> perimeter;

  /// Starting point of measurement
  glm::vec3 startPoint{0.0f, 0.0f, 0.0f};

  /// Ending point of measurement
  glm::vec3 endPoint{0.0f, 0.0f, 0.0f};

  /// Current cursor position
  glm::vec3 cursorPosition{0.0f, 0.0f, 0.0f};

  /// Whether the data contains valid measurements
  bool isValid = false;
};

/**
 * @brief Widget for displaying measurement information as an overlay
 *
 * DESIGN RATIONALE:
 *
 * 1. Stateless Coordinator Pattern:
 *    - MeasurementDisplay does NOT store its own visibility state
 *    - Visibility is queried from UIFSMAdapter each frame
 *    - This follows the same pattern as other Phase 6 widgets
 *
 * 2. Measurement Data Storage:
 *    - Current measurement data is stored in currentData_
 *    - Data is updated via setMeasurementData() callback
 *    - MeasurementManager provides real-time measurement updates
 *
 * 3. Display Configuration:
 *    - Font size, position, and padding are configurable
 *    - Colors are defined for different measurement types
 *    - Uses ImGUI's GetBackgroundDrawList() for overlay rendering
 *
 * USAGE:
 *
 * 1. Create MeasurementDisplay with UIFSMAdapter and MeasurementManager
 * 2. Call setMeasurementData() when measurements change
 * 3. Call render() each frame to display the overlay
 * 4. Visibility is controlled through UIFSMAdapter
 */
class MeasurementDisplay {
public:
  /**
   * @brief Constructs a MeasurementDisplay widget
   * @param uiFSMAdapter Pointer to the UI FSM adapter (must not be null)
   * @param measurementManager Pointer to the measurement manager (must not be
   * null)
   *
   * @throws std::invalid_argument if either pointer is null
   */
  MeasurementDisplay(UIFSMAdapter *uiFSMAdapter,
                     MeasurementManager *measurementManager);

  /**
   * @brief Renders the measurement display overlay
   *
   * Queries visibility from UIFSMAdapter and only renders if visible.
   * Uses ImGUI's GetBackgroundDrawList() for overlay rendering.
   * Displays distance, angle, area, perimeter (if available).
   * Shows start point, end point, and cursor position coordinates.
   */
  void render();

  /**
   * @brief Updates the measurement data to display
   * @param data The new measurement data
   */
  void setMeasurementData(const MeasurementData &data);

  /**
   * @brief Gets the current measurement data
   * @return Reference to the current measurement data
   */
  [[nodiscard]] const MeasurementData &getMeasurementData() const;

private:
  /// Pointer to the UI FSM adapter (non-owning)
  UIFSMAdapter *uiFSMAdapter_;

  /// Pointer to the measurement manager (non-owning)
  MeasurementManager *measurementManager_;

  /// Current measurement data
  MeasurementData currentData_;

  /// Whether to show distance measurements
  bool showDistance_{true};

  /// Whether to show angle measurements
  bool showAngle_{true};

  /// Whether to show area measurements
  bool showArea_{true};

  /// Whether to show perimeter measurements
  bool showPerimeter_{true};

  /// Whether to show coordinate information
  bool showCoordinates_{true};

  /// Font size for display
  float fontSize_{14.0f};

  /// Display position (top-left corner offset)
  ImVec2 position_{10.0f, 10.0f};

  /// Padding around text
  ImVec2 padding_{8.0f, 8.0f};

  // ==========================================================================
  // Color Constants
  // ==========================================================================

  /// Distance measurement color (Yellow)
  static constexpr ImU32 COLOR_DISTANCE = 0xFFFF00FF;

  /// Angle measurement color (Cyan)
  static constexpr ImU32 COLOR_ANGLE = 0x00FFFFFF;

  /// Area measurement color (Magenta)
  static constexpr ImU32 COLOR_AREA = 0xFF00FFFF;

  /// Perimeter measurement color (Orange)
  static constexpr ImU32 COLOR_PERIMETER = 0xFFA500FF;

  /// Coordinates color (White)
  static constexpr ImU32 COLOR_COORDINATES = 0xFFFFFFFF;

  /// Background color (Semi-transparent black)
  static constexpr ImU32 COLOR_BACKGROUND = 0x80000000;

  // ==========================================================================
  // Helper Methods
  // ==========================================================================

  /**
   * @brief Renders a measurement line with colored text
   * @param drawList Pointer to the ImGUI draw list
   * @param position Position to render at (updated after rendering)
   * @param label The label text
   * @param value The formatted value
   * @param color The text color
   */
  void renderMeasurementLine(ImDrawList *drawList, ImVec2 &position,
                             const char *label, const char *value,
                             ImU32 color) const;

  /**
   * @brief Renders coordinate information
   * @param drawList Pointer to the ImGUI draw list
   * @param position Position to render at (updated after rendering)
   * @param label The label text
   * @param coord The coordinate value
   */
  void renderCoordinate(ImDrawList *drawList, ImVec2 &position,
                        const char *label, const glm::vec3 &coord) const;
};

} // namespace view
