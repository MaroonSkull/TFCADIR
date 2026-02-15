#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <optional>

#include "OpenGL/Details/Camera/Camera3D.hpp"
#include "View/WorkMode.hpp"

namespace model {
class SketchPlane;
} // namespace model

namespace view {

/**
 * @brief Controls camera positioning and state management for sketch mode
 * @details Implements the Memento pattern for saving and restoring camera
 * state. Provides methods to position the camera for optimal 2D sketching on
 *          predefined planes.
 *
 * Note: This class can operate in two modes:
 * 1. Internal camera mode: Uses its own Camera3D instance (default)
 * 2. External camera mode: Uses a pointer to an external Camera3D (set via
 *    setExternalCamera3D). This is used when the camera should be shared with
 *    RenderingPipeline3D for interactive 3D controls.
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
    float zoom{1.0f};
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
   * @details Updates both the internal state struct and the Camera3D object
   *          used for 3D rendering. This ensures View Presets and other
   *          camera operations affect the actual rendered view.
   */
  void setCurrentState(const CameraState &state);

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

  // ==========================================================================
  // 3D Camera Support Methods
  // ==========================================================================

  /**
   * @brief Gets the 3D camera instance
   * @return Reference to the Camera3D instance
   */
  Camera3D &getCamera3D() {
    return externalCamera3D_ ? *externalCamera3D_ : camera3D_;
  }

  /**
   * @brief Gets the 3D camera instance (const version)
   * @return Const reference to the Camera3D instance
   */
  const Camera3D &getCamera3D() const {
    return externalCamera3D_ ? *externalCamera3D_ : camera3D_;
  }

  /**
   * @brief Sets an external 3D camera to use instead of the internal one
   * @param camera Pointer to an external Camera3D instance
   * @details This is used to share a camera between CameraController and
   *          RenderingPipeline3D. When set, all camera operations will use
   *          the external camera instead of the internal one.
   */
  void setExternalCamera3D(Camera3D *camera) { externalCamera3D_ = camera; }

  /**
   * @brief Checks if an external camera is set
   * @return True if using an external camera, false otherwise
   */
  bool hasExternalCamera3D() const { return externalCamera3D_ != nullptr; }

  /**
   * @brief Sets the working mode for the camera
   * @param mode The working mode (2D or 3D)
   */
  void setWorkMode(WorkMode mode);

  /**
   * @brief Gets the current working mode
   * @return The current working mode
   */
  WorkMode getWorkMode() const { return workMode_; }

  /**
   * @brief Gets the view matrix for the current mode
   * @return The view matrix
   */
  glm::mat4 getViewMatrix() const;

  /**
   * @brief Gets the projection matrix for the current mode
   * @param aspectRatio The aspect ratio of the viewport
   * @return The projection matrix
   */
  glm::mat4 getProjectionMatrix(float aspectRatio) const;

private:
  /// Saved camera state from before entering sketch mode
  std::optional<CameraState> savedState_;

  /// Current camera state
  CameraState currentState_;

  /// 3D camera instance for orbit navigation
  Camera3D camera3D_;

  /// Pointer to external 3D camera (used when sharing with RenderingPipeline3D)
  Camera3D *externalCamera3D_{nullptr};

  /// Current working mode
  WorkMode workMode_ = WorkMode::TwoD;

  /// Default distance from camera to sketch plane (world units)
  static constexpr float DEFAULT_CAMERA_DISTANCE = 10.0f;
};

} // namespace view
