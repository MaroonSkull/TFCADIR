#pragma once

#include <Model/IModel.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/Precision/GridManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace view {

/**
 * @brief Snap mode enumeration for different snap types
 *
 * Note: Tangent and Perpendicular snap modes are defined but not yet
 * implemented. These are planned for future enhancement and require:
 * - Tangent: Calculate tangent lines to curves/circles at snap points
 * - Perpendicular: Calculate perpendicular lines from cursor to geometry
 */
enum class SnapMode {
  Grid,
  Endpoint,
  Midpoint,
  Center,
  Intersection,
  Nearest,
  Tangent,      // TODO: Future enhancement - not yet implemented
  Perpendicular // TODO: Future enhancement - not yet implemented
};

/**
 * @brief Represents a single snap point with metadata
 *
 * SnapPoint contains all information needed for snap point calculation,
 * visualization, and selection. The distance field is in screen pixels
 * for consistent tolerance checking regardless of zoom level.
 */
struct SnapPoint {
  /// World position of the snap point
  glm::vec3 position;

  /// Snap mode that produced this point
  SnapMode mode;

  /// Human-readable description (e.g., "Endpoint", "Grid Point")
  std::string description;

  /// ID of the figure this snap point belongs to (0 for grid points)
  uint32_t figureId;

  /// Distance from cursor in pixels (for sorting and tolerance checking)
  float distance;

  /// Default constructor
  SnapPoint();

  /// Full constructor with all fields
  SnapPoint(const glm::vec3 &pos, SnapMode m, const std::string &desc,
            uint32_t id = 0, float dist = 0.0f);
};

/**
 * @brief Manager for object snapping functionality
 *
 * Provides snap point calculation for various snap modes:
 * - Grid snap
 * - Endpoint snap
 * - Midpoint snap
 * - Center snap
 * - Intersection snap
 * - Nearest snap
 * - Tangent snap (future)
 * - Perpendicular snap (future)
 *
 * Uses hybrid cache invalidation strategy (event-based + distance-based)
 * for optimal performance and correctness.
 */
class SnapManager {
public:
  /// Set of snap modes for per-tool configuration
  using SnapModeSet = std::unordered_set<SnapMode>;

  /**
   * @brief Construct SnapManager
   * @param uiFSMAdapter UIFSMAdapter reference for accessing snap settings
   * @param gridManager GridManager reference for grid snapping
   * @param selectionManager SelectionManager reference for object snapping
   * @param model IModel reference for accessing figure data
   */
  SnapManager(UIFSMAdapter &uiFSMAdapter, GridManager &gridManager,
              SelectionManager &selectionManager, model::IModel &model);

  /**
   * @brief Destroy SnapManager and clean up event callbacks
   */
  ~SnapManager();

  // Query methods (stateless)

  /**
   * @brief Check if snap is enabled for specific mode
   * @param snapMode Snap mode to check
   * @return true if snap is enabled
   */
  bool isSnapEnabled(SnapMode snapMode) const;

  /**
   * @brief Get snap tolerance in pixels
   * @return Tolerance in pixels
   */
  float getSnapTolerance() const;

  /**
   * @brief Get snap indicator color
   * @return Snap indicator color
   */
  glm::vec4 getIndicatorColor() const;

  /**
   * @brief Check if visual indicators should be shown
   * @return true if indicators should be shown
   */
  bool showIndicators() const;

  // Global snap enable/disable (for snap blocking)

  /**
   * @brief Enable or disable snapping globally
   * @param enabled true to enable snap, false to disable
   * @note When disabled, all snap modes are blocked regardless of settings
   */
  void setSnapEnabled(bool enabled);

  /**
   * @brief Check if snapping is enabled globally
   * @return true if snap is enabled
   */
  bool isSnapEnabled() const;

  // Snap calculation methods

  /**
   * @brief Find all snap points for the given screen position
   * @param screenPos Screen position in pixels
   * @param worldPos World position corresponding to screen position
   * @return Vector of all available snap points
   *
   * Uses hybrid cache invalidation (event-based + distance-based)
   * for optimal performance. Returns all snap points within tolerance,
   * sorted by distance from cursor.
   */
  std::vector<SnapPoint> findSnapPoints(const glm::vec2 &screenPos,
                                        const glm::vec3 &worldPos);

  /**
   * @brief Get the nearest snap point within tolerance
   * @param screenPos Screen position in pixels
   * @param worldPos World position corresponding to screen position
   * @return Nearest snap point if within tolerance, std::nullopt otherwise
   *
   * Finds the single closest snap point from all available snap points.
   * Only returns snap points within the configured tolerance.
   */
  std::optional<SnapPoint> getNearestSnapPoint(const glm::vec2 &screenPos,
                                               const glm::vec3 &worldPos);

  // Snap mode implementations

  /**
   * @brief Snap position to grid
   * @param worldPos World position to snap
   * @return Snap point if grid snap is within tolerance, std::nullopt otherwise
   */
  std::optional<SnapPoint> snapToGrid(const glm::vec3 &worldPos);

  /**
   * @brief Find all endpoint snap points for given figures
   * @param figureIds Vector of figure IDs to process
   * @return Vector of endpoint snap points
   */
  std::vector<SnapPoint>
  snapToEndpoints(const std::vector<uint32_t> &figureIds);

  /**
   * @brief Find all midpoint snap points for given figures
   * @param figureIds Vector of figure IDs to process
   * @return Vector of midpoint snap points
   */
  std::vector<SnapPoint>
  snapToMidpoints(const std::vector<uint32_t> &figureIds);

  /**
   * @brief Find all center snap points for given figures
   * @param figureIds Vector of figure IDs to process
   * @return Vector of center snap points
   */
  std::vector<SnapPoint> snapToCenters(const std::vector<uint32_t> &figureIds);

  /**
   * @brief Find all intersection snap points for given figures
   * @param figureIds Vector of figure IDs to process
   * @return Vector of intersection snap points
   */
  std::vector<SnapPoint>
  snapToIntersections(const std::vector<uint32_t> &figureIds);

  // Performance caching

  /**
   * @brief Invalidate the snap point cache
   * @note Called automatically by event handlers
   * Can also be called manually if needed
   */
  void invalidateCache();

private:
  /// Reference to UIFSMAdapter for accessing snap settings
  UIFSMAdapter &uiFSMAdapter_;

  /// Reference to GridManager for grid snapping
  GridManager &gridManager_;

  /// Reference to SelectionManager for object snapping
  SelectionManager &selectionManager_;

  /// Reference to IModel for accessing figure data
  model::IModel &model_;

  /// Global snap enable/disable flag
  bool snapEnabled_ = true;

  // Performance cache (transient, not FSM state)

  /**
   * @brief Snap point cache for performance optimization
   *
   * Cache is invalidated by:
   * - Event-based: Figure modification, camera changes, settings changes
   * - Distance-based: Cursor moved more than CACHE_INVALIDATION_DISTANCE pixels
   */
  struct SnapCache {
    /// Cached snap points from last calculation
    std::vector<SnapPoint> snapPoints;

    /// Screen position of last cache update
    glm::vec2 lastScreenPos{0.0f, 0.0f};

    /// Figure IDs used for last cache update
    std::vector<uint32_t> cachedFigureIds;

    /// Whether cache is valid (not invalidated by events)
    bool valid = false;
  } snapCache_;

  /// Distance threshold for distance-based cache invalidation (pixels)
  static constexpr float CACHE_INVALIDATION_DISTANCE = 5.0f;

  // Event handler callbacks (for cache invalidation)

  /// Callback for figure modification events
  std::function<void()> figureEventCallback_;

  /// Callback for camera change events
  std::function<void()> cameraEventCallback_;

  /// Callback for settings change events
  std::function<void()> settingsEventCallback_;

  // Event handler implementations

  /**
   * @brief Handle figure modification events
   * Invalidates cache when figures are added, removed, or modified
   */
  void onFigureModified();

  /**
   * @brief Handle camera change events
   * Invalidates cache when camera zoom, pan, or orbit changes
   */
  void onCameraChanged();

  /**
   * @brief Handle settings change events
   * Invalidates cache when grid or snap settings change
   */
  void onSettingsChanged();

  // Helper methods

  /**
   * @brief Calculate all snap points for the given position
   * @param screenPos Screen position in pixels
   * @param worldPos World position corresponding to screen position
   * @return Vector of all available snap points
   */
  std::vector<SnapPoint> calculateSnapPoints(const glm::vec2 &screenPos,
                                             const glm::vec3 &worldPos);

  /**
   * @brief Calculate distance squared between two points (optimized for
   * comparison)
   * @param a First point
   * @param b Second point
   * @return Distance squared
   */
  static float distanceSquared(const glm::vec3 &a, const glm::vec3 &b);

  /**
   * @brief Project world position onto line segment
   * @param point Point to project
   * @param lineStart Line segment start
   * @param lineEnd Line segment end
   * @return Projected point on line segment
   */
  static glm::vec3 projectPointOnLine(const glm::vec3 &point,
                                      const glm::vec3 &lineStart,
                                      const glm::vec3 &lineEnd);

  /**
   * @brief Calculate intersection of two line segments (2D)
   * @param p1 First line start
   * @param p2 First line end
   * @param p3 Second line start
   * @param p4 Second line end
   * @return Intersection point if lines intersect, std::nullopt otherwise
   */
  static std::optional<glm::vec3> lineIntersection(const glm::vec3 &p1,
                                                   const glm::vec3 &p2,
                                                   const glm::vec3 &p3,
                                                   const glm::vec3 &p4);
};

} // namespace view
