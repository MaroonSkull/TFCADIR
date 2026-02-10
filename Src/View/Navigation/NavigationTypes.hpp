#pragma once

namespace view {

/**
 * @brief Navigation mode for tracking active navigation operations
 * @details This is UI-only operational tracking state, stored in NavigationManager,
 *          not in the FSM or UIFSMAdapter. It tracks which navigation operation
 *          is currently active for proper event handling.
 */
enum class NavigationMode {
  Idle,          ///< No active navigation operation
  Zooming,       ///< Zoom operation in progress (mouse wheel or keyboard)
  Panning2D,     ///< 2D pan operation in progress (middle mouse drag)
  Panning3D,     ///< 3D pan operation in progress (Shift + middle mouse drag)
  Orbiting,      ///< 3D orbit operation in progress (middle mouse drag)
  ZoomBox,       ///< Zoom box selection in progress (drag to define area)
  Transitioning  ///< View preset transition in progress (smooth animation)
};

/**
 * @brief Orbit center for 3D orbit operations
 * @details Defines the point around which the camera orbits during 3D navigation.
 *          This is domain state stored in UIFSMAdapter.
 */
enum class OrbitCenter {
  Origin,    ///< Orbit around world origin (0, 0, 0)
  Selection, ///< Orbit around center of selected figures
  Custom     ///< Orbit around custom user-defined point
};

/**
 * @brief View preset for predefined camera positions
 * @details Defines standard CAD view presets for quick camera positioning.
 *          This is domain state stored in UIFSMAdapter.
 */
enum class ViewPreset {
  // 2D presets
  Top2D,   ///< 2D top view (default for sketch mode)

  // 3D presets
  Top3D,     ///< Top view (looking down Y axis)
  Front,     ///< Front view (looking toward +Z)
  Right,     ///< Right view (looking toward +X)
  Bottom,    ///< Bottom view (looking up Y axis)
  Back,      ///< Back view (looking toward -Z)
  Left,      ///< Left view (looking toward -X)
  Isometric, ///< Isometric view
  Custom     ///< Custom view angle
};

} // namespace view
