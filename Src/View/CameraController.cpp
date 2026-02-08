#include "CameraController.hpp"

#include "Model/SketchPlane.hpp"

namespace view {

void CameraController::saveCameraState(const glm::vec3 &position,
                                       const glm::vec3 &target,
                                       const glm::vec3 &up,
                                       const glm::mat4 &projection,
                                       const glm::mat4 &model) {
  savedState_ = CameraState{position, target, up, projection, model};
}

void CameraController::restoreCameraState() {
  if (!savedState_.has_value()) {
    return;
  }

  // Apply the saved state directly to the controller's current state
  currentState_ = savedState_.value();
  savedState_.reset();
}

CameraController::CameraState
CameraController::setCameraForPlane(const model::SketchPlane &plane) {
  // Get the camera view matrix from the sketch plane
  glm::mat4 viewMatrix = plane.getCameraViewMatrix();

  // Extract position, target, and up from the view matrix
  // The view matrix is glm::lookAt(eye, center, up)
  // We need to decompose it to get these values back

  // For simplicity, we'll use the plane's properties directly
  // Camera is positioned along the plane's normal at DEFAULT_CAMERA_DISTANCE
  glm::vec3 cameraPosition =
      plane.getOrigin() + plane.getNormal() * DEFAULT_CAMERA_DISTANCE;

  // Target is the plane origin
  glm::vec3 cameraTarget = plane.getOrigin();

  // Up is the plane's up vector
  glm::vec3 cameraUp = plane.getUp();

  // Create an orthographic projection for 2D sketching
  // This provides a flat, undistorted view of the sketch plane
  // TODO: Adjust bounds based on viewport size and desired work area
  constexpr float orthoSize = 20.0f;
  glm::mat4 projection =
      glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);

  // Identity model matrix for sketching (no additional transformations)
  glm::mat4 model = glm::mat4(1.0f);

  return CameraState{cameraPosition, cameraTarget, cameraUp, projection, model};
}

} // namespace view
