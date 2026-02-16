#include "Camera3D.hpp"

#include <algorithm>
#include <cmath>

namespace view {

Camera3D::Camera3D()
    : position_{0.0f, 0.0f, 5.0f}, target_{0.0f, 0.0f, 0.0f},
      up_{0.0f, 1.0f, 0.0f}, right_{1.0f, 0.0f, 0.0f},
      forward_{0.0f, 0.0f, -1.0f}, fov_{DEFAULT_FOV}, nearPlane_{DEFAULT_NEAR},
      farPlane_{DEFAULT_FAR}, minDistance_{DEFAULT_MIN_DISTANCE},
      maxDistance_{DEFAULT_MAX_DISTANCE} {
  updateCameraVectors();
}

Camera3D::Camera3D(const glm::vec3 &position, const glm::vec3 &target)
    : position_{position}, target_{target}, up_{0.0f, 1.0f, 0.0f},
      right_{1.0f, 0.0f, 0.0f}, forward_{0.0f, 0.0f, -1.0f}, fov_{DEFAULT_FOV},
      nearPlane_{DEFAULT_NEAR}, farPlane_{DEFAULT_FAR},
      minDistance_{DEFAULT_MIN_DISTANCE}, maxDistance_{DEFAULT_MAX_DISTANCE} {
  updateCameraVectors();
}

void Camera3D::rotate(float deltaX, float deltaY) {
  // Calculate current distance from target
  float distance = glm::length(position_ - target_);

  // Calculate current spherical coordinates
  glm::vec3 offset = position_ - target_;

  // Horizontal angle (yaw) - rotation around Y axis
  float yaw = std::atan2(offset.x, offset.z);

  // Vertical angle (pitch) - rotation around X axis
  float horizontalDist = std::sqrt(offset.x * offset.x + offset.z * offset.z);
  float pitch = std::atan2(offset.y, horizontalDist);

  // Apply rotation
  yaw += deltaX;
  pitch += deltaY;

  // Clamp pitch to prevent camera flip at poles
  constexpr float maxPitch = glm::radians(89.0f);
  constexpr float minPitch = glm::radians(-89.0f);
  pitch = std::clamp(pitch, minPitch, maxPitch);

  // Convert back to Cartesian coordinates
  offset.x = distance * std::sin(yaw) * std::cos(pitch);
  offset.y = distance * std::sin(pitch);
  offset.z = distance * std::cos(yaw) * std::cos(pitch);

  // Update position
  position_ = target_ + offset;

  // Update camera vectors
  updateCameraVectors();
}

void Camera3D::lookAt(const glm::vec3 &target) {
  target_ = target;
  updateCameraVectors();
}

void Camera3D::setPosition(const glm::vec3 &position) {
  position_ = position;
  updateCameraVectors();
}

void Camera3D::setTarget(const glm::vec3 &target) {
  target_ = target;
  updateCameraVectors();
}

void Camera3D::setUp(const glm::vec3 &up) {
  /// Validate input vector - ignore zero vectors to prevent NaN
  if (glm::length(up) < EPSILON) {
    return;
  }

  up_ = glm::normalize(up);

  /// Calculate right vector and check for parallel vectors
  glm::vec3 right = glm::cross(forward_, up_);
  if (glm::length(right) < EPSILON) {
    /// forward_ and up_ are parallel - don't update right_
    return;
  }
  right_ = glm::normalize(right);
}

void Camera3D::zoom(float delta) {
  // Calculate direction from target to camera
  glm::vec3 direction = position_ - target_;
  float distance = glm::length(direction);

  // Apply zoom (move camera along the direction)
  float newDistance = distance - delta;

  // Clamp distance to limits
  newDistance = std::clamp(newDistance, minDistance_, maxDistance_);

  // Update position
  if (distance > 0.001f) {
    glm::vec3 normalizedDir = direction / distance;
    position_ = target_ + normalizedDir * newDistance;
  }

  // Camera vectors remain the same direction
}

void Camera3D::pan(float deltaX, float deltaY) {
  // Move both camera and target in screen-space right and up directions
  glm::vec3 offset = right_ * deltaX + up_ * deltaY;
  position_ += offset;
  target_ += offset;

  // Camera vectors remain the same
}

glm::mat4 Camera3D::getViewMatrix() const {
  return glm::lookAt(position_, target_, up_);
}

glm::mat4 Camera3D::getProjectionMatrix(float aspectRatio) const {
  return glm::perspective(glm::radians(fov_), aspectRatio, nearPlane_,
                          farPlane_);
}

void Camera3D::setFOV(float fov) { fov_ = std::clamp(fov, 1.0f, 180.0f); }

void Camera3D::setPlanes(float nearPlane, float farPlane) {
  nearPlane_ = std::max(0.001f, nearPlane);
  farPlane_ = std::max(nearPlane_ + 0.1f, farPlane);
}

void Camera3D::setZoomLimits(float minDistance, float maxDistance) {
  minDistance_ = std::max(0.1f, minDistance);
  maxDistance_ = std::max(minDistance_ + 0.1f, maxDistance);
}

void Camera3D::updateCameraVectors() {
  /// Calculate forward vector (direction from camera to target)
  forward_ = glm::normalize(target_ - position_);

  /// Calculate right vector (perpendicular to forward and world up)
  /// Handle edge case when forward is parallel to WORLD_UP (Top/Bottom views)
  glm::vec3 right = glm::cross(forward_, WORLD_UP);

  if (glm::length(right) < EPSILON) {
    /// forward_ is parallel to WORLD_UP - use alternative reference vector
    /// This happens for Top view (looking down) or Bottom view (looking up)
    right = glm::cross(forward_, glm::vec3(1.0f, 0.0f, 0.0f));
    if (glm::length(right) < EPSILON) {
      /// Fallback: forward is also parallel to X axis, use Z axis
      right = glm::cross(forward_, glm::vec3(0.0f, 0.0f, 1.0f));
    }
  }
  right_ = glm::normalize(right);

  /// Calculate up vector (perpendicular to forward and right)
  up_ = glm::normalize(glm::cross(right_, forward_));
}

} // namespace view
