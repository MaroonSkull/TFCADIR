#pragma once

#include "Camera3d.hpp"

#include <glm/glm.hpp>

// Forward declaration for glfw::Window
namespace glfw {
class Window;
}

namespace view {

/**
 * @brief Manages mouse/keyboard input for orbit camera controls
 * @details Handles orbit rotation, pan, and zoom operations through
 *          glfwpp event system. Integrates with Camera3D for actual
 *          camera manipulation.
 */
class OrbitControls {
public:
  /**
   * @brief Constructs orbit controls with a camera reference
   * @param camera Reference to the Camera3D to control
   */
  explicit OrbitControls(Camera3D &camera);

  /**
   * @brief Destructor
   */
  ~OrbitControls() = default;

  // ==========================================================================
  // Input Handling
  // ==========================================================================

  /**
   * @brief Handles mouse scroll for zoom
   * @param yOffset Vertical scroll offset
   * @details Zooms camera in/out based on scroll direction.
   *          Positive yOffset zooms in, negative zooms out.
   */
  void onScroll(double yOffset);

  /**
   * @brief Handles keyboard input for camera control
   * @param key Key code pressed
   * @param action Key action (press, release, repeat)
   * @param mods Modifier keys (shift, ctrl, alt)
   * @details Supports:
   *          - Arrow keys: Orbit rotation
   *          - W/S: Zoom in/out
   *          - A/D: Pan left/right
   *          - R: Reset view
   */
  void onKey(int key, int action, int mods);

  /**
   * @brief Handles mouse cursor movement for orbit/pan
   * @param xpos Current cursor X position
   * @param ypos Current cursor Y position
   * @details When left mouse button is held, orbits camera.
   *          When middle mouse button is held, pans camera.
   */
  void onCursorPos(double xpos, double ypos);

  /**
   * @brief Handles mouse button press/release
   * @param button Mouse button pressed
   * @param action Button action (press, release)
   * @param mods Modifier keys
   * @details Tracks button state for cursor movement handling.
   */
  void onMouseButton(int button, int action, int mods);

  // ==========================================================================
  // Configuration
  // ==========================================================================

  /**
   * @brief Sets the rotation sensitivity
   * @param sensitivity Rotation speed multiplier
   */
  void setRotationSensitivity(float sensitivity);

  /**
   * @brief Sets the pan sensitivity
   * @param sensitivity Pan speed multiplier
   */
  void setPanSensitivity(float sensitivity);

  /**
   * @brief Sets the zoom sensitivity
   * @param sensitivity Zoom speed multiplier
   */
  void setZoomSensitivity(float sensitivity);

  /**
   * @brief Resets the camera to default view
   * @details Resets camera position and target to default values.
   */
  void resetView();

  // ==========================================================================
  // State Getters
  // ==========================================================================

  /**
   * @brief Checks if left mouse button is currently pressed
   * @return true if left mouse button is pressed
   */
  [[nodiscard]] bool isLeftMousePressed() const { return leftMousePressed_; }

  /**
   * @brief Checks if middle mouse button is currently pressed
   * @return true if middle mouse button is pressed
   */
  [[nodiscard]] bool isMiddleMousePressed() const {
    return middleMousePressed_;
  }

private:
  /// Reference to the controlled camera
  Camera3D &camera_;

  /// Last cursor X position for delta calculation
  double lastCursorX_{0.0};

  /// Last cursor Y position for delta calculation
  double lastCursorY_{0.0};

  /// Whether left mouse button is currently pressed
  bool leftMousePressed_{false};

  /// Whether middle mouse button is currently pressed
  bool middleMousePressed_{false};

  /// Rotation sensitivity (radians per pixel)
  float rotationSensitivity_{0.005f};

  /// Pan sensitivity (world units per pixel)
  float panSensitivity_{0.01f};

  /// Zoom sensitivity (world units per scroll unit)
  float zoomSensitivity_{1.0f};

  /// Default camera position for reset
  static constexpr glm::vec3 DEFAULT_POSITION{0.0f, 5.0f, 10.0f};

  /// Default camera target for reset
  static constexpr glm::vec3 DEFAULT_TARGET{0.0f, 0.0f, 0.0f};
};

} // namespace view
