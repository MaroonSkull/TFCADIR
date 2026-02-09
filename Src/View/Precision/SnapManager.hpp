#pragma once

#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace view {

/**
 * @file SnapManager.hpp
 * @brief Manages snap settings and snap point detection for precision drawing
 *
 * Architecture Notes:
 *
 * SnapManager is NOT purely stateless - it maintains a dirty flag for caching.
 * This architectural decision is justified for the following reasons:
 *
 * 1. Performance: Snap point detection is expensive (requires geometric
 * calculations)
 * 2. Caching: The dirty flag allows UI panels to cache snap point detection
 * results
 * 3. Integration: The dirty flag is set via callback when snap settings change
 *
 * This pattern is similar to GridManager's grid geometry caching.
 *
 * The stateless coordinator pattern is maintained for:
 * - Snap settings queries (getSnapSettings, isSnapModeEnabled, etc.)
 * - Snap point detection (findSnapPoint, findGridSnapPoint, etc.)
 *
 * Only the dirty flag is stateful, and it's explicitly managed through
 * callbacks.
 *
 * @see GridManager for similar pattern
 */

/**
 * @brief Snap mode enumeration
 *
 * Defines the available snap modes for precision drawing. Each mode
 * corresponds to a specific type of snap point that can be detected.
 */
enum class SnapMode {
  Grid = 1 << 0,          ///< Snap to grid intersections
  Endpoint = 1 << 1,      ///< Snap to figure endpoints
  Midpoint = 1 << 2,      ///< Snap to line/curve midpoints
  Center = 1 << 3,        ///< Snap to circle/arc centers
  Intersection = 1 << 4,  ///< Snap to line intersections
  Nearest = 1 << 5,       ///< Snap to nearest point on figure
  Tangent = 1 << 6,       ///< Snap to tangent points
  Perpendicular = 1 << 7, ///< Snap to perpendicular points
  All = Grid | Endpoint | Midpoint | Center | Intersection | Nearest | Tangent |
        Perpendicular
};

/**
 * @brief Snap point structure
 *
 * Contains information about a detected snap point, including its
 * position, the snap mode that detected it, and a human-readable description.
 */
struct SnapPoint {
  /// Position of the snap point in world coordinates
  glm::vec2 position;

  /// The snap mode that detected this point
  SnapMode mode;

  /// Human-readable description of the snap point
  std::string description;
};

/**
 * @brief Snap manager for precision drawing operations
 *
 * SnapManager provides snap functionality for precise drawing operations.
 * It manages snap settings, provides snap point detection, and maintains
 * a dirty flag for caching optimization.
 *
 * Architecture:
 * - SnapManager is NOT purely stateless - it maintains a dirty flag for caching
 * - This is acceptable because snap point detection is expensive (requires
 * geometric calculations)
 * - This pattern is similar to GridManager's grid geometry caching
 * - The dirty flag is set via callback when snap settings change in
 * UIFSMAdapter
 *
 * @note UI panels should query snap state through UIFSMAdapter, not directly
 */
class SnapManager {
public:
  /**
   * @brief Constructor
   * @param uiFSMAdapter Reference to UIFSMAdapter for integration
   *
   * The UIFSMAdapter reference is stored for state queries. SnapManager
   * does not manage the lifecycle of UIFSMAdapter.
   */
  explicit SnapManager(UIFSMAdapter &uiFSMAdapter);

  /**
   * @brief Destructor
   */
  ~SnapManager() = default;

  // ==========================================================================
  // State Query Methods (for SnapSettingsPanel)
  // These methods delegate to UIFSMAdapter for state queries
  // ==========================================================================

  /**
   * @brief Get current snap settings
   * @return Current snap settings
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] SnapSettings getSnapSettings() const;

  /**
   * @brief Check if grid snap is enabled
   * @return true if grid snap is enabled
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isGridSnapEnabled() const;

  /**
   * @brief Check if endpoint snap is enabled
   * @return true if endpoint snap is enabled
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isEndpointSnapEnabled() const;

  /**
   * @brief Check if midpoint snap is enabled
   * @return true if midpoint snap is enabled
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isMidpointSnapEnabled() const;

  /**
   * @brief Check if center snap is enabled
   * @return true if center snap is enabled
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isCenterSnapEnabled() const;

  /**
   * @brief Check if intersection snap is enabled
   * @return true if intersection snap is enabled
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isIntersectionSnapEnabled() const;

  /**
   * @brief Check if nearest point snap is enabled
   * @return true if nearest point snap is enabled
   * @note Queries UIFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isNearestSnapEnabled() const;

  /**
   * @brief Check if tangent snap is enabled
   * @return true if tangent snap is enabled
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isTangentSnapEnabled() const;

  /**
   * @brief Check if perpendicular snap is enabled
   * @return true if perpendicular snap is enabled
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] bool isPerpendicularSnapEnabled() const;

  /**
   * @brief Get snap tolerance
   * @return Snap tolerance in pixels
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] float getSnapTolerance() const;

  /**
   * @brief Check if snap indicators should be shown
   * @return true if indicators should be shown
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] bool showSnapIndicators() const;

  /**
   * @brief Get snap indicator color
   * @return Snap indicator color (RGBA)
   * @note Queries UIFFSMAdapter for the snap settings
   */
  [[nodiscard]] glm::vec4 getSnapIndicatorColor() const;

  // ==========================================================================
  // Dirty Flag Mechanism (for caching snap point detection)
  // ==========================================================================

  /**
   * @brief Check if snap settings have changed since last check
   * @return true if settings are dirty
   *
   * Snap settings are marked dirty when snap settings change.
   * This allows efficient caching of snap point detection results.
   */
  [[nodiscard]] bool isSnapSettingsDirty() const;

  /**
   * @brief Clear the snap settings dirty flag
   *
   * Call this after regenerating snap point detection results to indicate
   * that the cached data is up to date.
   */
  void clearSnapSettingsDirty();

  // ==========================================================================
  // Snap Point Detection Methods (for drawing operations)
  // ==========================================================================

  /**
   * @brief Find snap point near the given position
   * @param position Position to search near (in screen coordinates)
   * @param figures List of figures to search for snap points
   * @return Snap point if found, empty optional if not found
   *
   * Searches for snap points based on enabled snap modes. Returns the
   * closest snap point within tolerance, or empty optional if none found.
   */
  [[nodiscard]] std::optional<SnapPoint>
  findSnapPoint(const glm::vec2 &position,
                const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Find snap point for grid snap only
   * @param position Position to search near (in screen coordinates)
   * @param gridSettings Grid settings for grid snap calculation
   * @return Snap point if found, empty optional if not found
   *
   * Calculates the nearest grid intersection point to the given position.
   */
  [[nodiscard]] std::optional<SnapPoint>
  findGridSnapPoint(const glm::vec2 &position,
                    const GridSettings &gridSettings);

private:
  /**
   * @name State Caching
   * @brief SnapManager maintains caching state for performance optimization
   * @details
   * SnapManager is NOT purely stateless - it maintains a dirty flag for
   * caching. This is an intentional architectural decision for performance:
   * - Snap point detection is expensive (requires geometric calculations)
   * - Snap settings change infrequently compared to frame rate
   * - Caching avoids recalculating snap points every frame
   *
   * The dirty flag is set via callback from UIFSMAdapter when settings change.
   * This pattern is similar to GridManager's grid geometry caching.
   */
  ///@{
  /// Reference to UIFSMAdapter (non-owning)
  UIFSMAdapter &uiFSMAdapter_;

  /// Flag to track if snap settings have changed
  mutable bool snapSettingsDirty_;
  ///@}

  /// Logger for diagnostic output
  std::shared_ptr<spdlog::logger> logger_;

  // ==========================================================================
  // Helper Methods for Snap Point Detection
  // ==========================================================================

  /**
   * @brief Find endpoint snap points
   * @param position Position to search near
   * @param figures List of figures to search
   * @return Snap point if found, empty optional if not found
   */
  [[nodiscard]] std::optional<SnapPoint>
  findEndpointSnap(const glm::vec2 &position,
                   const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Find midpoint snap points
   * @param position Position to search near
   * @param figures List of figures to search
   * @return Snap point if found, empty optional if not found
   */
  [[nodiscard]] std::optional<SnapPoint>
  findMidpointSnap(const glm::vec2 &position,
                   const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Find center snap points
   * @param position Position to search near
   * @param figures List of figures to search
   * @return Snap point if found, empty optional if not found
   */
  [[nodiscard]] std::optional<SnapPoint>
  findCenterSnap(const glm::vec2 &position,
                 const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Find intersection snap points
   * @param position Position to search near
   * @param figures List of figures to search
   * @return Snap point if found, empty optional if not found
   */
  [[nodiscard]] std::optional<SnapPoint> findIntersectionSnap(
      const glm::vec2 &position,
      const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Find nearest snap point
   * @param position Position to search near
   * @param figures List of figures to search
   * @return Snap point if found, empty optional if not found
   */
  [[nodiscard]] std::optional<SnapPoint>
  findNearestSnap(const glm::vec2 &position,
                  const std::vector<std::shared_ptr<model::IFigure>> &figures);

  /**
   * @brief Calculate distance between two points
   * @param a First point
   * @param b Second point
   * @return Euclidean distance between the points
   */
  [[nodiscard]] float distance(const glm::vec2 &a, const glm::vec2 &b) const;
};

} // namespace view
