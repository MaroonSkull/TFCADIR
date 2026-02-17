#pragma once

namespace view {

/**
 * @brief Work mode enumeration for the application
 * @details Defines the working mode for camera, grid, and other view components
 */
enum class WorkMode {
  TwoD,  ///< 2D sketching mode
  ThreeD ///< 3D navigation mode
};

} // namespace view
