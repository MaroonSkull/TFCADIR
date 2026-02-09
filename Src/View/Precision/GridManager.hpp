#pragma once

#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace view {

/**
 * @brief Work mode enumeration for grid rendering
 */
enum class WorkMode { TwoD, ThreeD };

/**
 * @brief Grid geometry structure for rendering
 *
 * Contains vertex data for grid lines and their visual properties.
 * Used by rendering components to display the grid on screen.
 */
struct GridGeometry {
  /// Major grid line vertices (pairs of points defining lines)
  std::vector<glm::vec3> majorLines;

  /// Minor grid line vertices (pairs of points defining lines)
  std::vector<glm::vec3> minorLines;

  /// Spacing between major grid lines
  float majorSpacing;

  /// Spacing between minor grid lines
  float minorSpacing;

  /// Grid color (major lines)
  glm::vec4 color;

  /// Minor grid color
  glm::vec4 minorColor;

  /// Grid opacity (0.0 to 1.0)
  float opacity;
};

/**
 * @brief Axis geometry structure for rendering
 *
 * Contains vertex data for coordinate axes and their colors.
 * Used by rendering components to display X, Y, Z axes.
 */
struct AxisGeometry {
  /// X axis line vertices (start and end points)
  std::vector<glm::vec3> xAxis;

  /// Y axis line vertices (start and end points)
  std::vector<glm::vec3> yAxis;

  /// Z axis line vertices (start and end points, 3D only)
  std::vector<glm::vec3> zAxis;

  /// X axis color (typically red)
  glm::vec4 xColor;

  /// Y axis color (typically green)
  glm::vec4 yColor;

  /// Z axis color (typically blue)
  glm::vec4 zColor;

  /// Whether to show Z axis (true for 3D, false for 2D)
  bool showZ;
};

/**
 * @brief Grid rendering manager for 2D/3D CAD canvas
 *
 * Manages grid visualization for the CAD canvas. Queries grid settings
 * from UIFSMAdapter and calculates grid geometry for rendering.
 * Follows the stateless coordinator pattern - no local state storage.
 *
 * GridManager is responsible for:
 * - Querying grid settings from UIFSMAdapter (stateless)
 * - Calculating grid geometry based on settings
 * - Providing grid data for rendering
 * - Handling 2D/3D mode differences
 */
class GridManager {
public:
  /**
   * @brief Construct a new Grid Manager object
   * @param uiFSMAdapter Reference to UIFSMAdapter for state queries
   *
   * The UIFSMAdapter reference is stored for state queries. GridManager
   * does not manage the lifecycle of UIFSMAdapter.
   */
  GridManager(UIFSMAdapter &uiFSMAdapter);

  /**
   * @brief Destroy the Grid Manager object
   */
  ~GridManager() = default;

  /**
   * @brief Get grid geometry for rendering
   * @param mode Current work mode (2D or 3D)
   * @return GridGeometry Structure containing grid vertices and parameters
   *
   * Queries grid settings from UIFSMAdapter and calculates the
   * appropriate grid geometry based on the current work mode.
   */
  GridGeometry getGridGeometry(WorkMode mode) const;

  /**
   * @brief Get axis geometry for rendering
   * @param mode Current work mode (2D or 3D)
   * @return AxisGeometry Structure containing axis vertices and colors
   *
   * Returns axis geometry with X and Y axes for 2D mode,
   * and X, Y, Z axes for 3D mode.
   */
  AxisGeometry getAxisGeometry(WorkMode mode) const;

  /**
   * @brief Check if grid is visible
   * @return true if grid should be rendered
   *
   * Queries the grid visibility setting from UIFSMAdapter.
   */
  bool isGridVisible() const;

  /**
   * @brief Check if axes are visible
   * @return true if axes should be rendered
   *
   * Queries the axes visibility setting from UIFSMAdapter.
   */
  bool areAxesVisible() const;

  /**
   * @brief Check if origin indicator is visible
   * @return true if origin should be rendered
   *
   * Queries the origin visibility setting from UIFSMAdapter.
   */
  bool isOriginVisible() const;

  /**
   * @brief Get grid color
   * @return glm::vec4 Grid color (major lines)
   *
   * Queries the grid color setting from UIFSMAdapter.
   */
  glm::vec4 getGridColor() const;

  /**
   * @brief Get minor grid color
   * @return glm::vec4 Minor grid color
   *
   * Queries the minor grid color setting from UIFSMAdapter.
   */
  glm::vec4 getMinorGridColor() const;

  /**
   * @brief Get grid opacity
   * @return float Grid opacity (0.0 to 1.0)
   *
   * Queries the grid opacity setting from UIFSMAdapter.
   */
  float getGridOpacity() const;

  /**
   * @brief Convert world position to grid coordinates
   * @param worldPos World position (3D)
   * @return glm::vec2 Grid coordinates (2D)
   *
   * Converts a world space position to grid space coordinates.
   * For 2D mode, uses X and Y components. For 3D mode, uses X and Z components.
   */
  glm::vec2 worldToGrid(const glm::vec3 &worldPos) const;

  /**
   * @brief Convert grid coordinates to world position
   * @param gridPos Grid coordinates (2D)
   * @return glm::vec3 World position (3D, Z=0 for 2D mode)
   *
   * Converts grid space coordinates to world space position.
   * For 2D mode, returns position with Z=0. For 3D mode, returns position with
   * Y=0.
   */
  glm::vec3 gridToWorld(const glm::vec2 &gridPos) const;

  /**
   * @brief Snap world position to nearest grid point
   * @param worldPos World position to snap
   * @return glm::vec3 Snapped world position
   *
   * Rounds the world position to the nearest grid intersection
   * based on the current grid spacing setting.
   */
  glm::vec3 snapToGrid(const glm::vec3 &worldPos) const;

private:
  /// Reference to UIFSMAdapter (non-owning)
  UIFSMAdapter &uiFSMAdapter_;

  /**
   * @brief Calculate 2D grid geometry
   * @param settings Grid settings from UIFSMAdapter
   * @return GridGeometry Calculated grid vertices
   *
   * Generates horizontal and vertical lines for 2D drafting mode.
   * Grid is centered at origin (0, 0).
   */
  GridGeometry calculate2DGrid(const GridSettings &settings) const;

  /**
   * @brief Calculate 3D grid geometry
   * @param settings Grid settings from UIFSMAdapter
   * @return GridGeometry Calculated grid vertices
   *
   * Generates ground plane grid (XZ plane) for 3D modeling mode.
   * Includes vertical grid lines at intervals.
   */
  GridGeometry calculate3DGrid(const GridSettings &settings) const;
};

} // namespace view
