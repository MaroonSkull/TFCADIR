#pragma once

#include <View/CameraController.hpp>
#include <View/Navigation/NavigationTypes.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace view {

// Forward declaration
class SelectionManager;

/**
 * @brief Stateless coordinator for navigation operations
 * @details NavigationManager provides a convenient API for navigation
 * operations without storing local domain state (following SelectionManager
 * pattern). All domain navigation state is queried from and managed by
 * UIFSMAdapter, which serves as the single source of truth. NavigationMode and
 *          TransitionState are stored as UI-only operational tracking state.
 */
class NavigationManager {
public:
  /**
   * @brief Construct a NavigationManager
   * @param fsmAdapter Reference to UIFSMAdapter for navigation state queries
   * @param camera Reference to CameraController for camera state manipulation
   * @param selectionManager Optional pointer to SelectionManager for orbit
   * center calculation
   */
  NavigationManager(UIFSMAdapter &fsmAdapter, CameraController &camera,
                    SelectionManager *selectionManager = nullptr);

  /**
   * @brief Destructor
   */
  ~NavigationManager() = default;

  // ==========================================================================
  // Query Methods (stateless - delegate to UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Get the current navigation mode (UI-only state)
   * @return Current navigation mode
   */
  NavigationMode getNavigationMode() const;

  /**
   * @brief Get the current orbit center
   * @return Current orbit center type
   * @note Queries UIFSMAdapter for the orbit center state
   */
  OrbitCenter getOrbitCenter() const;

  /**
   * @brief Get the current view preset
   * @return Current view preset
   * @note Queries UIFSMAdapter for the view preset state
   */
  ViewPreset getCurrentViewPreset() const;

  /**
   * @brief Check if a view transition is in progress
   * @return true if a transition is in progress
   * @note Queries UIFSMAdapter for the transition state
   */
  bool isTransitioning() const;

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
  // Zoom Operations (stateless - pass all state as parameters)
  // ==========================================================================

  /**
   * @brief Zoom at cursor position
   * @param delta Zoom delta (positive = zoom in, negative = zoom out)
   * @param cursorPos Current cursor position in screen coordinates
   * @param viewportSize Viewport size in pixels
   */
  void zoomAtCursor(float delta, const glm::ivec2 &cursorPos,
                    const glm::ivec2 &viewportSize);

  /**
   * @brief Zoom in by a fixed amount
   */
  void zoomIn();

  /**
   * @brief Zoom out by a fixed amount
   */
  void zoomOut();

  /**
   * @brief Zoom to a specific zoom level
   * @param zoomLevel Target zoom level
   */
  void zoomToPreset(float zoomLevel);

  /**
   * @brief Zoom to fit all objects in view
   */
  void zoomToFit();

  /**
   * @brief Zoom to fit selected objects in view
   */
  void zoomToSelection();

  /**
   * @brief Start zoom box selection
   * @param startPos Starting position of the zoom box
   */
  void startZoomBox(const glm::ivec2 &startPos);

  /**
   * @brief Update zoom box during drag
   * @param currentPos Current cursor position
   */
  void updateZoomBox(const glm::ivec2 &currentPos);

  /**
   * @brief End zoom box and apply zoom
   */
  void endZoomBox();

  // ==========================================================================
  // Pan Operations (stateless - pass all state as parameters)
  // ==========================================================================

  /**
   * @brief Start pan operation
   * @param startPos Starting position of the pan
   */
  void startPan(const glm::ivec2 &startPos);

  /**
   * @brief Update pan during drag
   * @param currentPos Current cursor position
   * @param startPos Starting position of the pan
   */
  void updatePan(const glm::ivec2 &currentPos, const glm::ivec2 &startPos);

  /**
   * @brief End pan operation
   */
  void endPan();

  // ==========================================================================
  // Orbit Operations (stateless - pass all state as parameters)
  // ==========================================================================

  /**
   * @brief Start orbit operation
   * @param startPos Starting position of the orbit
   */
  void startOrbit(const glm::ivec2 &startPos);

  /**
   * @brief Update orbit during drag
   * @param currentPos Current cursor position
   * @param startPos Starting position of the orbit
   * @param orbitCenter Point to orbit around
   */
  void updateOrbit(const glm::ivec2 &currentPos, const glm::ivec2 &startPos,
                   const glm::vec3 &orbitCenter);

  /**
   * @brief End orbit operation
   */
  void endOrbit();

  // ==========================================================================
  // View Preset Operations
  // ==========================================================================

  /**
   * @brief Set a view preset
   * @param preset The view preset to set
   */
  void setViewPreset(ViewPreset preset);

  /**
   * @brief Cycle to next view preset
   */
  void nextViewPreset();

  /**
   * @brief Cycle to previous view preset
   */
  void previousViewPreset();

  // ==========================================================================
  // View Transition Operations (UI-only state management)
  // ==========================================================================

  /**
   * @brief Start a view transition animation
   * @param targetPreset Target view preset
   * @param duration Transition duration in seconds
   */
  void startTransition(ViewPreset targetPreset, float duration = 0.3f);

  /**
   * @brief Update view transition animation
   * @param deltaTime Time elapsed since last update in seconds
   * @return true if transition is still in progress
   */
  bool updateTransition(float deltaTime);

  // ==========================================================================
  // Orbit Center Management
  // ==========================================================================

  /**
   * @brief Set the orbit center type
   * @param center The orbit center type
   */
  void setOrbitCenter(OrbitCenter center);

  /**
   * @brief Set a custom orbit center point
   * @param center The custom orbit center coordinates
   */
  void setCustomOrbitCenter(const glm::vec3 &center);

  /**
   * @brief Calculate orbit center based on current settings
   * @return Calculated orbit center point
   */
  glm::vec3 calculateOrbitCenter() const;

  // ==========================================================================
  // Validation (DEBUG builds only)
  // ==========================================================================

#ifndef NDEBUG
  /**
   * @brief Validate navigation state consistency
   * @return true if navigation state is valid
   */
  bool validateNavigationState() const;
#endif

private:
  /// Reference to UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Reference to CameraController (no ownership)
  CameraController &camera_;

  /// Optional pointer to SelectionManager for orbit center calculation (no
  /// ownership)
  SelectionManager *selectionManager_;

  // ==========================================================================
  // UI-only state (NOT in FSM or UIFSMAdapter)
  // ==========================================================================

  /// Current navigation mode (operational tracking)
  NavigationMode currentNavigationMode_ = NavigationMode::Idle;

  /// View transition state (transient, operational tracking)
  struct TransitionState {
    bool isActive = false;
    float progress = 0.0f;
    float duration = 0.3f;
    glm::vec3 startPos{0.0f, 0.0f, 0.0f};
    glm::vec3 startTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 startUp{0.0f, 1.0f, 0.0f};
    glm::vec3 endPos{0.0f, 0.0f, 0.0f};
    glm::vec3 endTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 endUp{0.0f, 1.0f, 0.0f};
  } transitionState_;

  /// Pan start position for tracking pan operation
  glm::ivec2 panStartPos_;

  /// Zoom box start position for tracking zoom box operation
  glm::ivec2 zoomBoxStartPos_;

  // ==========================================================================
  // Helper Methods
  // ==========================================================================

  /**
   * @brief Get camera state for a view preset
   * @param preset The view preset
   * @return Camera state (position, target, up)
   */
  struct CameraPresetState {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
  };

  CameraPresetState getCameraPresetState(ViewPreset preset) const;

  /**
   * @brief Apply smooth easing function for transitions
   * @param t Progress value (0.0 to 1.0)
   * @return Eased value
   */
  float easeInOutCubic(float t) const;

  // NO controller ownership (following SelectionManager pattern exactly)
  // NO local domain state (UIFSMAdapter is single source of truth)
};

} // namespace view
