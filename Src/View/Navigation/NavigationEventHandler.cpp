#include <View/Navigation/NavigationEventHandler.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <View/Tools/ToolManager.hpp>
#include <spdlog/spdlog.h>

namespace view {

NavigationEventHandler::NavigationEventHandler(
    NavigationManager &navigationManager, ToolManager *toolManager)
    : navigationManager_(navigationManager), toolManager_(toolManager),
      lastCursorPos_(0, 0), startCursorPos_(0, 0), isMiddleMousePressed_(false),
      isLeftMousePressed_(false), isNavigating_(false) {}

bool NavigationEventHandler::handleMouseWheel(float delta,
                                              const glm::ivec2 &cursorPos,
                                              const glm::ivec2 &viewportSize) {
  /// Check if navigation is allowed by current tool
  if (!isNavigationAllowed()) {
    return false;
  }

  /// Handle zoom at cursor position
  navigationManager_.zoomAtCursor(delta, cursorPos, viewportSize);
  return true;
}

bool NavigationEventHandler::handleMouseButtonPress(
    int button, int mods, const glm::ivec2 &cursorPos) {
  /// Check if navigation is allowed by current tool
  if (!isNavigationAllowed()) {
    return false;
  }

  /// Handle middle mouse button (button 2)
  if (button == 2) {
    isMiddleMousePressed_ = true;
    startCursorPos_ = cursorPos;
    lastCursorPos_ = cursorPos;

    /// Determine navigation mode based on current view
    NavigationMode mode = determineNavigationMode();

    if (mode == NavigationMode::Orbiting) {
      /// Start orbit in 3D mode
      navigationManager_.startOrbit(cursorPos);
    } else {
      /// Start pan in 2D or 3D mode
      navigationManager_.startPan(cursorPos);
    }

    isNavigating_ = true;
    return true;
  }

  /// Handle left mouse button (button 0) for zoom box
  if (button == 0 && mods == 0) {
    isLeftMousePressed_ = true;
    startCursorPos_ = cursorPos;
    /// Zoom box is started when mouse is released
    return true;
  }

  return false;
}

bool NavigationEventHandler::handleMouseButtonRelease(
    int button, const glm::ivec2 &cursorPos) {
  /// Handle middle mouse button release
  if (button == 2 && isMiddleMousePressed_) {
    isMiddleMousePressed_ = false;

    /// End navigation operation
    NavigationMode currentMode = navigationManager_.getNavigationMode();
    if (currentMode == NavigationMode::Orbiting) {
      navigationManager_.endOrbit();
    } else if (currentMode == NavigationMode::Panning2D ||
               currentMode == NavigationMode::Panning3D) {
      navigationManager_.endPan();
    }

    isNavigating_ = false;
    return true;
  }

  /// Handle left mouse button release for zoom box
  if (button == 0 && isLeftMousePressed_) {
    isLeftMousePressed_ = false;

    /// Check if mouse moved significantly (zoom box)
    glm::ivec2 delta = cursorPos - startCursorPos_;
    if (glm::length(glm::vec2(delta)) > 5.0f) {
      /// Significant movement - treat as zoom box
      navigationManager_.startZoomBox(startCursorPos_);
      navigationManager_.updateZoomBox(cursorPos);
      navigationManager_.endZoomBox();
    }

    return true;
  }

  return false;
}

bool NavigationEventHandler::handleMouseMove(const glm::ivec2 &cursorPos) {
  /// Only handle if currently navigating
  if (!isNavigating_ || !isMiddleMousePressed_) {
    lastCursorPos_ = cursorPos;
    return false;
  }

  /// Calculate delta from start position
  glm::ivec2 delta = cursorPos - startCursorPos_;

  /// Update navigation based on current mode
  NavigationMode currentMode = navigationManager_.getNavigationMode();

  if (currentMode == NavigationMode::Orbiting) {
    /// Update orbit - calculate orbit center based on current settings
    glm::vec3 orbitCenter = navigationManager_.calculateOrbitCenter();
    navigationManager_.updateOrbit(cursorPos, startCursorPos_, orbitCenter);
  } else if (currentMode == NavigationMode::Panning2D ||
             currentMode == NavigationMode::Panning3D) {
    /// Update pan
    navigationManager_.updatePan(cursorPos, startCursorPos_);
  } else if (currentMode == NavigationMode::ZoomBox) {
    /// Update zoom box visualization
    navigationManager_.updateZoomBox(cursorPos);
  }

  lastCursorPos_ = cursorPos;
  return true;
}

bool NavigationEventHandler::handleKeyPress(int key, int mods) {
  /// Handle keyboard shortcuts for view presets
  if (mods == 0) {
    switch (key) {
    case 'T': /// Top view
      navigationManager_.setViewPreset(ViewPreset::Top3D);
      return true;

    case 'F': /// Front view
      navigationManager_.setViewPreset(ViewPreset::Front);
      return true;

    case 'R': /// Right view
      navigationManager_.setViewPreset(ViewPreset::Right);
      return true;

    case 'I': /// Isometric view
      navigationManager_.setViewPreset(ViewPreset::Isometric);
      return true;

    case 'H': /// Home/Top 2D view
      navigationManager_.setViewPreset(ViewPreset::Top2D);
      return true;

    default:
      break;
    }
  }

  /// Handle modifier key combinations
  if (mods == 1) { /// Shift key
    switch (key) {
    case 'T': /// Bottom view
      navigationManager_.setViewPreset(ViewPreset::Bottom);
      return true;

    case 'F': /// Back view
      navigationManager_.setViewPreset(ViewPreset::Back);
      return true;

    case 'R': /// Left view
      navigationManager_.setViewPreset(ViewPreset::Left);
      return true;

    default:
      break;
    }
  }

  return false;
}

bool NavigationEventHandler::isNavigating() const { return isNavigating_; }

NavigationMode NavigationEventHandler::getNavigationMode() const {
  return navigationManager_.getNavigationMode();
}

bool NavigationEventHandler::isNavigationAllowed() const {
  /// If no tool manager is set, allow navigation
  if (!toolManager_) {
    return true;
  }

  /// Check if a tool is currently active
  /// Navigation is only allowed when no tool is active
  std::string activeTool = toolManager_->getActiveTool();
  return activeTool.empty();
}

NavigationMode NavigationEventHandler::determineNavigationMode() const {
  /// Determine navigation mode based on current view preset
  ViewPreset currentPreset = navigationManager_.getCurrentViewPreset();

  if (currentPreset == ViewPreset::Top2D) {
    /// 2D mode - pan only
    return NavigationMode::Panning2D;
  } else {
    /// 3D mode - check if shift is held for pan vs orbit
    /// For now, default to orbit in 3D
    return NavigationMode::Orbiting;
  }
}

} // namespace view
