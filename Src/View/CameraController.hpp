#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <optional>

namespace model {
class SketchPlane;
} // namespace model

namespace view {

/**
 * @brief Controls camera positioning and state management for sketch mode
 * @details Implements the Memento pattern for saving and restoring camera
 * state. Provides methods to position the camera for optimal 2D sketching on
 *          predefined planes.
 */
class CameraController {
public:
  /**
   * @brief Saved camera state for memento pattern
   */
  struct CameraState {
    glm::vec3 position{0.0f, 0.0f, -1.0f};
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::mat4 projection{1.0f};
    glm::mat4 model{1.0f};
  };

  /**
   * @brief Constructs a new CameraController
   */
  CameraController() = default;

  /**
   * @brief Destructor
   */
  ~CameraController() = default;

  /**
   * @brief Saves the current camera state
   * @param position Current camera position in world space
   * @param target Current camera target/look-at point in world space
   * @param up Current camera up vector
   * @param projection Current projection matrix
   * @param model Current model matrix
   * @details Stores the current camera state internally for later restoration.
   *          This is called when entering sketch mode to preserve the 3D view.
   */
  void saveCameraState(const glm::vec3 &position, const glm::vec3 &target,
                       const glm::vec3 &up, const glm::mat4 &projection,
                       const glm::mat4 &model);

  /**
   * @brief Restores the previously saved camera state
   * @details This is called when exiting sketch mode to return to the 3D view.
   *          Applies the saved state directly to the controller's current
   * state.
   * @note After restoring, the saved state is cleared.
   */
  void restoreCameraState();

  /**
   * @brief Gets the current camera state
   * @return The current camera state
   */
  const CameraState &getCurrentState() const { return currentState_; }

  /**
   * @brief Sets the current camera state
   * @param state The camera state to set
   */
  void setCurrentState(const CameraState &state) { currentState_ = state; }

  /**
   * @brief Positions the camera for optimal 2D sketching on the given plane
   * @param plane The sketch plane to position the camera for
   * @return The camera state (position, target, up) for the plane
   * @details Calculates an orthographic view of the sketch plane with the
   *          camera positioned perpendicular to the plane at a suitable
   * distance. The up vector is aligned with the plane's up direction.
   */
  CameraState setCameraForPlane(const model::SketchPlane &plane);

  /**
   * @brief Checks if a saved camera state exists
   * @return true if a state has been saved and not yet restored
   */
  bool hasSavedState() const { return savedState_.has_value(); }

  /**
   * @brief Clears the saved camera state without restoring
   * @details Use this to discard the saved state (e.g., when resetting)
   */
  void clearSavedState() { savedState_.reset(); }

  // ==========================================================================
  // Phase 5: Navigation Support Methods
  // ==========================================================================

  /**
   * @brief Set camera state with individual parameters
   * @param position Camera position in world space
   * @param target Camera target/look-at point in world space
   * @param up Camera up vector
   * @param zoom Zoom level
   */
  void setCameraState(const glm::vec3 &position, const glm::vec3 &target,
                      const glm::vec3 &up, float zoom);

  /**
   * @brief Get camera state as individual parameters
   * @param position Output camera position
   * @param target Output camera target
   * @param up Output camera up vector
   * @param zoom Output zoom level
   */
  void getCameraState(glm::vec3 &position, glm::vec3 &target, glm::vec3 &up,
                      float &zoom) const;

  /**
   * @brief Check if camera is in 2D mode
   * @return true if camera is in 2D mode
   */
  bool is2DMode() const;

  /**
   * @brief Check if camera is in 3D mode
   * @return true if camera is in 3D mode
   */
  bool is3DMode() const;

private:
  /// Saved camera state from before entering sketch mode
  std::optional<CameraState> savedState_;

  /// Current camera state
  CameraState currentState_;

  /// Flag to track if camera is in 2D mode
  bool is2DMode_ = true;

  /// Default distance from camera to sketch plane (world units)
  static constexpr float DEFAULT_CAMERA_DISTANCE = 10.0f;
};

} // namespace view
