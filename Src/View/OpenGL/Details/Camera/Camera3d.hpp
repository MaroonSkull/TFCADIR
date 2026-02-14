#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace view {

/**
 * @brief Manages 3D camera state and provides camera control operations
 * @details Implements orbit camera controls for 3D navigation including
 *          rotation around a target, zoom, and pan operations.
 */
class Camera3D {
public:
  /**
   * @brief Constructs a default 3D camera
   * @details Initializes camera at position (0, 0, 5) looking at origin
   */
  Camera3D();

  /**
   * @brief Constructs a camera with specified position and target
   * @param position Initial camera position in world space
   * @param target Initial look-at target in world space
   */
  Camera3D(const glm::vec3 &position, const glm::vec3 &target);

  /**
   * @brief Destructor
   */
  ~Camera3D() = default;

  // ==========================================================================
  // Camera Control Methods
  // ==========================================================================

  /**
   * @brief Rotates the camera around the target point
   * @param deltaX Horizontal rotation angle in radians
   * @param deltaY Vertical rotation angle in radians
   * @details Performs orbit rotation around the current target point.
   *          The camera maintains a constant distance from the target.
   */
  void rotate(float deltaX, float deltaY);

  /**
   * @brief Points the camera at a specific target
   * @param target The target position to look at
   * @details Updates the camera orientation to face the target while
   *          maintaining the current position.
   */
  void lookAt(const glm::vec3 &target);

  /**
   * @brief Sets the camera position in world space
   * @param position The new camera position
   * @details Updates camera position and recalculates view matrix.
   */
  void setPosition(const glm::vec3 &position);

  /**
   * @brief Sets the target point the camera looks at
   * @param target The new target position
   * @details Updates the look-at target and recalculates camera vectors.
   */
  void setTarget(const glm::vec3 &target);

  /**
   * @brief Zooms the camera in or out
   * @param delta Zoom amount (positive to zoom in, negative to zoom out)
   * @details Adjusts the distance between camera and target.
   *          Clamped to minimum and maximum distances.
   */
  void zoom(float delta);

  /**
   * @brief Pans the camera horizontally and vertically
   * @param deltaX Horizontal pan amount in world units
   * @param deltaY Vertical pan amount in world units
   * @details Moves both camera and target together in screen-space
   *          right and up directions.
   */
  void pan(float deltaX, float deltaY);

  // ==========================================================================
  // Matrix Getters
  // ==========================================================================

  /**
   * @brief Gets the view matrix for rendering
   * @return The view matrix transforming world space to view space
   */
  [[nodiscard]] glm::mat4 getViewMatrix() const;

  /**
   * @brief Gets the projection matrix for rendering
   * @param aspectRatio The viewport aspect ratio (width / height)
   * @return The projection matrix for perspective projection
   */
  [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

  // ==========================================================================
  // State Getters
  // ==========================================================================

  /**
   * @brief Gets the camera position
   * @return Current camera position in world space
   */
  [[nodiscard]] const glm::vec3 &getPosition() const { return position_; }

  /**
   * @brief Gets the camera target
   * @return Current look-at target in world space
   */
  [[nodiscard]] const glm::vec3 &getTarget() const { return target_; }

  /**
   * @brief Gets the camera up vector
   * @return Current up direction
   */
  [[nodiscard]] const glm::vec3 &getUp() const { return up_; }

  /**
   * @brief Gets the camera right vector
   * @return Current right direction
   */
  [[nodiscard]] const glm::vec3 &getRight() const { return right_; }

  /**
   * @brief Gets the camera forward vector
   * @return Current forward direction (direction camera is facing)
   */
  [[nodiscard]] const glm::vec3 &getForward() const { return forward_; }

  /**
   * @brief Gets the field of view
   * @return Current FOV in degrees
   */
  [[nodiscard]] float getFOV() const { return fov_; }

  /**
   * @brief Gets the near plane distance
   * @return Near plane distance in world units
   */
  [[nodiscard]] float getNearPlane() const { return nearPlane_; }

  /**
   * @brief Gets the far plane distance
   * @return Far plane distance in world units
   */
  [[nodiscard]] float getFarPlane() const { return farPlane_; }

  // ==========================================================================
  // Configuration Setters
  // ==========================================================================

  /**
   * @brief Sets the field of view
   * @param fov Field of view in degrees (typically 30-120)
   */
  void setFOV(float fov);

  /**
   * @brief Sets the near and far plane distances
   * @param nearPlane Distance to near clipping plane
   * @param farPlane Distance to far clipping plane
   */
  void setPlanes(float nearPlane, float farPlane);

  /**
   * @brief Sets the zoom limits
   * @param minDistance Minimum distance from target
   * @param maxDistance Maximum distance from target
   */
  void setZoomLimits(float minDistance, float maxDistance);

private:
  /// Camera position in world space
  glm::vec3 position_;

  /// Target position the camera looks at
  glm::vec3 target_;

  /// Up direction (typically world up)
  glm::vec3 up_;

  /// Right direction (calculated from forward and up)
  glm::vec3 right_;

  /// Forward direction (direction camera is facing)
  glm::vec3 forward_;

  /// Field of view in degrees
  float fov_;

  /// Near clipping plane distance
  float nearPlane_;

  /// Far clipping plane distance
  float farPlane_;

  /// Minimum zoom distance from target
  float minDistance_;

  /// Maximum zoom distance from target
  float maxDistance_;

  /// World up vector (used for calculating right vector)
  static constexpr glm::vec3 WORLD_UP{0.0f, 1.0f, 0.0f};

  /// Default field of view in degrees
  static constexpr float DEFAULT_FOV = 45.0f;

  /// Default near plane distance
  static constexpr float DEFAULT_NEAR = 0.1f;

  /// Default far plane distance
  static constexpr float DEFAULT_FAR = 1000.0f;

  /// Default minimum zoom distance
  static constexpr float DEFAULT_MIN_DISTANCE = 1.0f;

  /// Default maximum zoom distance
  static constexpr float DEFAULT_MAX_DISTANCE = 500.0f;

  /**
   * @brief Updates camera vectors based on position and target
   * @details Recalculates forward, right, and up vectors after
   *          position or target changes.
   */
  void updateCameraVectors();
};

} // namespace view
