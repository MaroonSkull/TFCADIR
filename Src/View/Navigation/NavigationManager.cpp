#include <View/Navigation/NavigationManager.hpp>

#include <View/ObjectManagement/SelectionManager.hpp>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace view {

NavigationManager::NavigationManager(UIFSMAdapter &fsmAdapter,
                                     CameraController &camera,
                                     SelectionManager *selectionManager)
    : fsmAdapter_(fsmAdapter), camera_(camera),
      selectionManager_(selectionManager) {}

// ==========================================================================
// Query Methods (stateless - delegate to UIFSMAdapter)
// ==========================================================================

NavigationMode NavigationManager::getNavigationMode() const {
  return currentNavigationMode_;
}

OrbitCenter NavigationManager::getOrbitCenter() const {
  return fsmAdapter_.getOrbitCenter();
}

ViewPreset NavigationManager::getCurrentViewPreset() const {
  return fsmAdapter_.getCurrentViewPreset();
}

bool NavigationManager::isTransitioning() const {
  return fsmAdapter_.isTransitioning();
}

bool NavigationManager::is2DMode() const {
  /// Check if current view preset is 2D
  ViewPreset preset = getCurrentViewPreset();
  return preset == ViewPreset::Top2D;
}

bool NavigationManager::is3DMode() const {
  /// Check if current view preset is 3D
  return !is2DMode();
}

// ==========================================================================
// Zoom Operations (stateless - pass all state as parameters)
// ==========================================================================

void NavigationManager::zoomAtCursor(float delta, const glm::ivec2 &cursorPos,
                                     const glm::ivec2 &viewportSize) {
  /// Calculate zoom factor based on delta
  float zoomFactor = 1.0f + (delta * 0.1f);

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate direction from camera to target
  glm::vec3 viewDirection = currentState.target - currentState.position;
  float currentDistance = glm::length(viewDirection);
  glm::vec3 normalizedDirection = glm::normalize(viewDirection);

  /// Calculate new distance after zoom
  float newDistance = currentDistance / zoomFactor;

  /// Clamp distance to prevent zooming too close or too far
  newDistance = glm::clamp(newDistance, 1.0f, 1000.0f);

  /// Calculate new camera position
  glm::vec3 newPosition =
      currentState.target - normalizedDirection * newDistance;

  /// Update camera state
  CameraController::CameraState newState;
  newState.position = newPosition;
  newState.target = currentState.target;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::zoomIn() {
  /// Zoom in by a fixed amount (20%)
  float zoomFactor = 1.2f;

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate direction from camera to target
  glm::vec3 viewDirection = currentState.target - currentState.position;
  float currentDistance = glm::length(viewDirection);
  glm::vec3 normalizedDirection = glm::normalize(viewDirection);

  /// Calculate new distance after zoom
  float newDistance = currentDistance / zoomFactor;

  /// Clamp distance to prevent zooming too close
  newDistance = glm::max(newDistance, 1.0f);

  /// Calculate new camera position
  glm::vec3 newPosition =
      currentState.target - normalizedDirection * newDistance;

  /// Update camera state
  CameraController::CameraState newState;
  newState.position = newPosition;
  newState.target = currentState.target;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::zoomOut() {
  /// Zoom out by a fixed amount (20%)
  float zoomFactor = 0.8f;

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate direction from camera to target
  glm::vec3 viewDirection = currentState.target - currentState.position;
  float currentDistance = glm::length(viewDirection);
  glm::vec3 normalizedDirection = glm::normalize(viewDirection);

  /// Calculate new distance after zoom
  float newDistance = currentDistance / zoomFactor;

  /// Clamp distance to prevent zooming too far
  newDistance = glm::min(newDistance, 1000.0f);

  /// Calculate new camera position
  glm::vec3 newPosition =
      currentState.target - normalizedDirection * newDistance;

  /// Update camera state
  CameraController::CameraState newState;
  newState.position = newPosition;
  newState.target = currentState.target;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::zoomToPreset(float zoomLevel) {
  /// Zoom to specific zoom level
  /// zoomLevel is a multiplier: 1.0 = current, 2.0 = 2x closer, 0.5 = 2x
  /// farther

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate direction from camera to target
  glm::vec3 viewDirection = currentState.target - currentState.position;
  float currentDistance = glm::length(viewDirection);
  glm::vec3 normalizedDirection = glm::normalize(viewDirection);

  /// Calculate new distance based on zoom level
  float newDistance = currentDistance / zoomLevel;

  /// Clamp distance to reasonable limits
  newDistance = glm::clamp(newDistance, 1.0f, 1000.0f);

  /// Calculate new camera position
  glm::vec3 newPosition =
      currentState.target - normalizedDirection * newDistance;

  /// Update camera state
  CameraController::CameraState newState;
  newState.position = newPosition;
  newState.target = currentState.target;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::zoomToFit() {
  /// Calculate bounding box of all objects
  /// Adjust camera to fit all objects in view
  /// TODO: Implement when model access is available
}

void NavigationManager::zoomToSelection() {
  if (!selectionManager_) {
    return;
  }

  /// Get selected figures
  auto selectedIds = selectionManager_->getSelectedFigureIds();
  if (selectedIds.empty()) {
    return;
  }

  /// Calculate bounding box of selection
  /// Adjust camera to fit selection in view
  /// TODO: Implement when model access is available
}

void NavigationManager::startZoomBox(const glm::ivec2 &startPos) {
  zoomBoxStartPos_ = startPos;
  currentNavigationMode_ = NavigationMode::ZoomBox;
}

void NavigationManager::updateZoomBox(const glm::ivec2 &currentPos) {
  /// Update zoom box visualization
  /// This would normally update the UI to show the zoom box
}

void NavigationManager::endZoomBox() {
  /// Calculate zoom box bounds
  /// Apply zoom to fit the box
  /// Reset navigation mode
  currentNavigationMode_ = NavigationMode::Idle;
}

// ==========================================================================
// Pan Operations (stateless - pass all state as parameters)
// ==========================================================================

void NavigationManager::startPan(const glm::ivec2 &startPos) {
  panStartPos_ = startPos;

  if (is2DMode()) {
    currentNavigationMode_ = NavigationMode::Panning2D;
  } else {
    currentNavigationMode_ = NavigationMode::Panning3D;
  }
}

void NavigationManager::updatePan(const glm::ivec2 &currentPos,
                                  const glm::ivec2 &startPos) {
  /// Calculate pan delta
  glm::vec2 delta = glm::vec2(currentPos - startPos);

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate pan speed based on zoom level
  float panSpeed = 0.01f;

  /// Update camera position and target
  glm::vec3 panOffset;
  if (is2DMode()) {
    /// In 2D mode, pan in X and Y
    panOffset = glm::vec3(delta.x * panSpeed, -delta.y * panSpeed, 0.0f);
  } else {
    /// In 3D mode, pan in camera's X and Y plane
    /// Calculate right and up vectors from camera state
    glm::vec3 right = glm::normalize(glm::cross(
        currentState.target - currentState.position, currentState.up));
    glm::vec3 up = currentState.up;

    panOffset = (right * delta.x - up * delta.y) * panSpeed;
  }

  /// Apply pan offset to camera position and target
  CameraController::CameraState newState;
  newState.position = currentState.position + panOffset;
  newState.target = currentState.target + panOffset;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::endPan() {
  /// Reset navigation mode
  currentNavigationMode_ = NavigationMode::Idle;
}

// ==========================================================================
// Orbit Operations (stateless - pass all state as parameters)
// ==========================================================================

void NavigationManager::startOrbit(const glm::ivec2 &startPos) {
  /// Only allow orbit in 3D mode
  if (!is3DMode()) {
    return;
  }

  panStartPos_ = startPos;
  currentNavigationMode_ = NavigationMode::Orbiting;
}

void NavigationManager::updateOrbit(const glm::ivec2 &currentPos,
                                    const glm::ivec2 &startPos,
                                    const glm::vec3 &orbitCenter) {
  /// Calculate orbit delta
  glm::vec2 delta = glm::vec2(currentPos - startPos);

  /// Calculate rotation angles
  float orbitSpeed = 0.01f;
  float horizontalAngle = delta.x * orbitSpeed;
  float verticalAngle = delta.y * orbitSpeed;

  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Calculate camera position relative to orbit center
  glm::vec3 relativePos = currentState.position - orbitCenter;
  float distance = glm::length(relativePos);

  /// Apply horizontal rotation (yaw)
  float cosH = glm::cos(horizontalAngle);
  float sinH = glm::sin(horizontalAngle);

  glm::vec3 tempPos;
  tempPos.x = relativePos.x * cosH - relativePos.z * sinH;
  tempPos.y = relativePos.y;
  tempPos.z = relativePos.x * sinH + relativePos.z * cosH;

  /// Apply vertical rotation (pitch)
  float cosV = glm::cos(verticalAngle);
  float sinV = glm::sin(verticalAngle);

  glm::vec3 newPos;
  newPos.x = tempPos.x;
  newPos.y = tempPos.y * cosV - tempPos.z * sinV;
  newPos.z = tempPos.y * sinV + tempPos.z * cosV;

  /// Update camera position (orbit center stays fixed)
  CameraController::CameraState newState;
  newState.position = orbitCenter + newPos;
  newState.target = orbitCenter;
  newState.up = currentState.up;
  newState.projection = currentState.projection;
  newState.model = currentState.model;

  camera_.setCurrentState(newState);
}

void NavigationManager::endOrbit() {
  /// Reset navigation mode
  currentNavigationMode_ = NavigationMode::Idle;
}

// ==========================================================================
// View Preset Operations
// ==========================================================================

void NavigationManager::setViewPreset(ViewPreset preset) {
  /// Get camera state for the preset
  CameraPresetState presetState = getCameraPresetState(preset);

  /// Apply camera state
  CameraController::CameraState cameraState;
  cameraState.position = presetState.position;
  cameraState.target = presetState.target;
  cameraState.up = presetState.up;

  camera_.setCurrentState(cameraState);

  /// Update view preset in UIFSMAdapter
  fsmAdapter_.setCurrentViewPreset(preset);
}

void NavigationManager::nextViewPreset() {
  ViewPreset current = getCurrentViewPreset();
  ViewPreset next;

  /// Cycle through presets
  switch (current) {
  case ViewPreset::Top2D:
    next = ViewPreset::Top3D;
    break;
  case ViewPreset::Top3D:
    next = ViewPreset::Front;
    break;
  case ViewPreset::Front:
    next = ViewPreset::Right;
    break;
  case ViewPreset::Right:
    next = ViewPreset::Bottom;
    break;
  case ViewPreset::Bottom:
    next = ViewPreset::Back;
    break;
  case ViewPreset::Back:
    next = ViewPreset::Left;
    break;
  case ViewPreset::Left:
    next = ViewPreset::Isometric;
    break;
  case ViewPreset::Isometric:
    next = ViewPreset::Top2D;
    break;
  default:
    next = ViewPreset::Top3D;
    break;
  }

  setViewPreset(next);
}

void NavigationManager::previousViewPreset() {
  ViewPreset current = getCurrentViewPreset();
  ViewPreset prev;

  /// Cycle through presets in reverse
  switch (current) {
  case ViewPreset::Top2D:
    prev = ViewPreset::Isometric;
    break;
  case ViewPreset::Top3D:
    prev = ViewPreset::Top2D;
    break;
  case ViewPreset::Front:
    prev = ViewPreset::Top3D;
    break;
  case ViewPreset::Right:
    prev = ViewPreset::Front;
    break;
  case ViewPreset::Bottom:
    prev = ViewPreset::Right;
    break;
  case ViewPreset::Back:
    prev = ViewPreset::Bottom;
    break;
  case ViewPreset::Left:
    prev = ViewPreset::Back;
    break;
  case ViewPreset::Isometric:
    prev = ViewPreset::Left;
    break;
  default:
    prev = ViewPreset::Top2D;
    break;
  }

  setViewPreset(prev);
}

// ==========================================================================
// View Transition Operations (UI-only state management)
// ==========================================================================

void NavigationManager::startTransition(ViewPreset targetPreset,
                                        float duration) {
  /// Get current camera state
  auto currentState = camera_.getCurrentState();

  /// Get target camera state
  CameraPresetState targetState = getCameraPresetState(targetPreset);

  /// Initialize transition state
  transitionState_.isActive = true;
  transitionState_.progress = 0.0f;
  transitionState_.duration = duration;
  transitionState_.startPos = currentState.position;
  transitionState_.startTarget = currentState.target;
  transitionState_.startUp = currentState.up;
  transitionState_.endPos = targetState.position;
  transitionState_.endTarget = targetState.target;
  transitionState_.endUp = targetState.up;

  /// Update navigation mode
  currentNavigationMode_ = NavigationMode::Transitioning;

  /// Update transitioning state in UIFSMAdapter
  fsmAdapter_.setIsTransitioning(true);
}

bool NavigationManager::updateTransition(float deltaTime) {
  if (!transitionState_.isActive) {
    return false;
  }

  /// Update progress
  transitionState_.progress += deltaTime / transitionState_.duration;

  if (transitionState_.progress >= 1.0f) {
    /// Transition complete
    transitionState_.progress = 1.0f;

    /// Apply final state
    CameraController::CameraState cameraState;
    cameraState.position = transitionState_.endPos;
    cameraState.target = transitionState_.endTarget;
    cameraState.up = transitionState_.endUp;

    camera_.setCurrentState(cameraState);

    /// Reset transition state
    transitionState_.isActive = false;
    currentNavigationMode_ = NavigationMode::Idle;

    /// Update transitioning state in UIFSMAdapter
    fsmAdapter_.setIsTransitioning(false);

    return false;
  }

  /// Apply easing function
  float t = easeInOutCubic(transitionState_.progress);

  /// Interpolate camera state
  CameraController::CameraState cameraState;
  cameraState.position =
      glm::mix(transitionState_.startPos, transitionState_.endPos, t);
  cameraState.target =
      glm::mix(transitionState_.startTarget, transitionState_.endTarget, t);
  cameraState.up =
      glm::mix(transitionState_.startUp, transitionState_.endUp, t);

  camera_.setCurrentState(cameraState);

  return true;
}

// ==========================================================================
// Orbit Center Management
// ==========================================================================

void NavigationManager::setOrbitCenter(OrbitCenter center) {
  fsmAdapter_.setOrbitCenter(center);
}

void NavigationManager::setCustomOrbitCenter(const glm::vec3 &center) {
  fsmAdapter_.setCustomOrbitCenter(center);
  /// Also set orbit center type to Custom
  fsmAdapter_.setOrbitCenter(OrbitCenter::Custom);
}

// ==========================================================================
// Validation (DEBUG builds only)
// ==========================================================================

#ifndef NDEBUG
bool NavigationManager::validateNavigationState() const {
  /// Validate navigation mode
  if (currentNavigationMode_ == NavigationMode::Orbiting && is2DMode()) {
    return false; /// Cannot orbit in 2D mode
  }

  /// Validate transition state
  if (transitionState_.isActive) {
    if (transitionState_.progress < 0.0f || transitionState_.progress > 1.0f) {
      return false;
    }
    if (transitionState_.duration <= 0.0f) {
      return false;
    }
  }

  return true;
}
#endif

// ==========================================================================
// Helper Methods
// ==========================================================================

glm::vec3 NavigationManager::calculateOrbitCenter() const {
  OrbitCenter centerType = fsmAdapter_.getOrbitCenter();

  switch (centerType) {
  case OrbitCenter::Origin:
    return glm::vec3(0.0f, 0.0f, 0.0f);

  case OrbitCenter::Selection:
    if (selectionManager_) {
      auto selectedIds = selectionManager_->getSelectedFigureIds();
      if (!selectedIds.empty()) {
        /// Calculate bounding box center of selection
        /// TODO: Implement when model access is available
        return glm::vec3(0.0f, 0.0f, 0.0f);
      }
    }
    return glm::vec3(0.0f, 0.0f, 0.0f);

  case OrbitCenter::Custom:
    return fsmAdapter_.getCustomOrbitCenter();

  default:
    return glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

NavigationManager::CameraPresetState
NavigationManager::getCameraPresetState(ViewPreset preset) const {
  CameraPresetState state;

  switch (preset) {
  case ViewPreset::Top2D:
    state.position = glm::vec3(0.0f, 0.0f, 100.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  case ViewPreset::Top3D:
    state.position = glm::vec3(0.0f, 100.0f, 0.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 0.0f, -1.0f);
    break;

  case ViewPreset::Front:
    state.position = glm::vec3(0.0f, 0.0f, 100.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  case ViewPreset::Right:
    state.position = glm::vec3(100.0f, 0.0f, 0.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  case ViewPreset::Bottom:
    state.position = glm::vec3(0.0f, -100.0f, 0.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 0.0f, 1.0f);
    break;

  case ViewPreset::Back:
    state.position = glm::vec3(0.0f, 0.0f, -100.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  case ViewPreset::Left:
    state.position = glm::vec3(-100.0f, 0.0f, 0.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  case ViewPreset::Isometric:
    state.position = glm::vec3(100.0f, 100.0f, 100.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;

  default:
    state.position = glm::vec3(0.0f, 0.0f, 100.0f);
    state.target = glm::vec3(0.0f, 0.0f, 0.0f);
    state.up = glm::vec3(0.0f, 1.0f, 0.0f);
    break;
  }

  return state;
}

float NavigationManager::easeInOutCubic(float t) const {
  /// Smooth cubic easing function
  return t < 0.5f ? 4.0f * t * t * t
                  : 1.0f - glm::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

} // namespace view
