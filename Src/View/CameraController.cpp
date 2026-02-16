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

  // Use setCurrentState() to update both currentState_ AND Camera3D
  setCurrentState(savedState_.value());
  savedState_.reset();
}

void CameraController::setCurrentState(const CameraState &state) {
  /// Apply new camera state for view preset or navigation operation
  currentState_ = state;

  /// Synchronize with Camera3D for 3D mode rendering
  /// This ensures View Presets affect the actual rendered view
  getCamera3D().setPosition(state.position);
  getCamera3D().setTarget(state.target);
  getCamera3D().setUp(state.up);
  /// Note: zoom is applied separately in getProjectionMatrix()
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

// ==========================================================================
// Navigation Support Methods
// ==========================================================================

void CameraController::setCameraState(const glm::vec3 &position,
                                      const glm::vec3 &target,
                                      const glm::vec3 &up, float zoom) {
  currentState_.position = position;
  currentState_.target = target;
  currentState_.up = up;
  currentState_.zoom = zoom;

  // Synchronize with Camera3D for 3D mode rendering
  getCamera3D().setPosition(position);
  getCamera3D().setTarget(target);
  getCamera3D().setUp(up);
  // Note: Camera3D uses fov_, not zoom - zoom is handled separately in
  // projection
}

void CameraController::getCameraState(glm::vec3 &position, glm::vec3 &target,
                                      glm::vec3 &up, float &zoom) const {
  position = currentState_.position;
  target = currentState_.target;
  up = currentState_.up;
  zoom = currentState_.zoom;
}

bool CameraController::is2DMode() const { return workMode_ == WorkMode::TwoD; }

bool CameraController::is3DMode() const {
  return workMode_ == WorkMode::ThreeD;
}

// ==========================================================================
// 3D Camera Support Methods
// ==========================================================================

void CameraController::setWorkMode(WorkMode mode) { workMode_ = mode; }

glm::mat4 CameraController::getViewMatrix() const {
  if (workMode_ == WorkMode::ThreeD) {
    return camera3D_.getViewMatrix();
  }
  // For 2D mode, use the current state to create view matrix
  return glm::lookAt(currentState_.position, currentState_.target,
                     currentState_.up);
}

glm::mat4 CameraController::getProjectionMatrix(float aspectRatio) const {
  if (workMode_ == WorkMode::ThreeD) {
    return camera3D_.getProjectionMatrix(aspectRatio);
  }
  // For 2D mode, return the stored projection matrix
  return currentState_.projection;
}

} // namespace view
