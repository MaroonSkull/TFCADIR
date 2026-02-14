#include "OrbitControls.hpp"

#include <algorithm>

// GLFW key codes
#ifndef GLFW_KEY_LEFT
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_W 87
#define GLFW_KEY_S 83
#define GLFW_KEY_A 65
#define GLFW_KEY_D 68
#define GLFW_KEY_R 82
#define GLFW_PRESS 1
#define GLFW_REPEAT 2
#define GLFW_RELEASE 0
#define GLFW_MOUSE_BUTTON_LEFT 0
#define GLFW_MOUSE_BUTTON_MIDDLE 2
#endif

namespace view {

OrbitControls::OrbitControls(Camera3D &camera) : camera_{camera} {}

void OrbitControls::onScroll(double yOffset) {
  // Zoom in for positive yOffset, out for negative
  camera_.zoom(static_cast<float>(yOffset) * zoomSensitivity_);
}

void OrbitControls::onKey(int key, int action, int /*mods*/) {
  // Only handle press and repeat
  if (action != GLFW_PRESS && action != GLFW_REPEAT) {
    return;
  }

  switch (key) {
  case GLFW_KEY_LEFT:
    // Orbit left
    camera_.rotate(-rotationSensitivity_ * 10.0f, 0.0f);
    break;
  case GLFW_KEY_RIGHT:
    // Orbit right
    camera_.rotate(rotationSensitivity_ * 10.0f, 0.0f);
    break;
  case GLFW_KEY_UP:
    // Orbit up
    camera_.rotate(0.0f, -rotationSensitivity_ * 10.0f);
    break;
  case GLFW_KEY_DOWN:
    // Orbit down
    camera_.rotate(0.0f, rotationSensitivity_ * 10.0f);
    break;
  case GLFW_KEY_W:
    // Zoom in
    camera_.zoom(zoomSensitivity_);
    break;
  case GLFW_KEY_S:
    // Zoom out
    camera_.zoom(-zoomSensitivity_);
    break;
  case GLFW_KEY_A:
    // Pan left
    camera_.pan(-panSensitivity_ * 10.0f, 0.0f);
    break;
  case GLFW_KEY_D:
    // Pan right
    camera_.pan(panSensitivity_ * 10.0f, 0.0f);
    break;
  case GLFW_KEY_R:
    // Reset view
    resetView();
    break;
  default:
    break;
  }
}

void OrbitControls::onCursorPos(double xpos, double ypos) {
  // Calculate delta from last position
  double deltaX = xpos - lastCursorX_;
  double deltaY = ypos - lastCursorY_;

  // Update last position
  lastCursorX_ = xpos;
  lastCursorY_ = ypos;

  // Handle based on mouse button state
  if (leftMousePressed_) {
    // Orbit camera
    camera_.rotate(static_cast<float>(deltaX) * rotationSensitivity_,
                   static_cast<float>(deltaY) * rotationSensitivity_);
  } else if (middleMousePressed_) {
    // Pan camera (inverted for natural feel)
    camera_.pan(-static_cast<float>(deltaX) * panSensitivity_,
                static_cast<float>(deltaY) * panSensitivity_);
  }
}

void OrbitControls::onMouseButton(int button, int action, int /*mods*/) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    leftMousePressed_ = (action == GLFW_PRESS);
  } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    middleMousePressed_ = (action == GLFW_PRESS);
  }
}

void OrbitControls::setRotationSensitivity(float sensitivity) {
  rotationSensitivity_ = std::max(0.0001f, sensitivity);
}

void OrbitControls::setPanSensitivity(float sensitivity) {
  panSensitivity_ = std::max(0.0001f, sensitivity);
}

void OrbitControls::setZoomSensitivity(float sensitivity) {
  zoomSensitivity_ = std::max(0.0001f, sensitivity);
}

void OrbitControls::resetView() {
  camera_.setPosition(DEFAULT_POSITION);
  camera_.setTarget(DEFAULT_TARGET);
}

} // namespace view
