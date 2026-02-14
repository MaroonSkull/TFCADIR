#pragma once

#include <View/Precision/SnapManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <optional>
#include <vector>

namespace view {

/**
 * @brief Visual representation of a snap indicator
 *
 * Contains all information needed to render a snap indicator on the canvas,
 * including position, type-specific icon/shape, and visual styling.
 */
struct SnapIndicatorVisual {
  /// Position of the snap indicator in screen coordinates
  glm::vec2 screenPosition;

  /// The snap mode that generated this indicator
  SnapMode snapMode;

  /// Whether this is the active (closest) snap point
  bool isActive{false};

  /// Size of the indicator in pixels
  float size{8.0f};

  /**
   * @brief Get the color for this indicator based on snap mode
   * @param baseColor The base indicator color from settings
   * @return Color to use for rendering this indicator
   */
  glm::vec4 getColor(const glm::vec4 &baseColor) const;
};

/**
 * @brief Canvas overlay for rendering snap indicators
 *
 * SnapIndicatorOverlay renders visual indicators on the canvas when snap
 * points are detected. It follows the stateless coordinator pattern,
 * querying state from UIFSMAdapter and SnapManager each frame.
 *
 * The overlay renders different shapes for different snap types:
 * - Grid: Small square
 * - Endpoint: Filled circle
 * - Midpoint: Triangle
 * - Center: Cross/plus
 * - Intersection: X mark
 * - Nearest: Diamond
 * - Tangent: Circle with tangent line
 * - Perpendicular: Right angle symbol
 */
class SnapIndicatorOverlay {
public:
  /**
   * @brief Construct a new Snap Indicator Overlay
   * @param uiFSMAdapter Reference to UIFSMAdapter for state queries
   * @param snapManager Reference to SnapManager for snap point detection
   */
  SnapIndicatorOverlay(UIFSMAdapter &uiFSMAdapter, SnapManager &snapManager);

  /**
   * @brief Destructor
   */
  ~SnapIndicatorOverlay() = default;

  /**
   * @brief Render snap indicators on the canvas
   * @param mousePosition Current mouse position in screen coordinates
   * @param canvasPosition Top-left corner of the canvas in screen coordinates
   * @param canvasSize Size of the canvas
   * @param figures List of figures to detect snap points from
   * @param gridSettings Grid settings for grid snap calculation
   *
   * Renders visual indicators for all detected snap points near the mouse
   * position. The closest snap point is highlighted as active.
   */
  void render(const glm::vec2 &mousePosition, const glm::vec2 &canvasPosition,
              const glm::vec2 &canvasSize,
              const std::vector<std::shared_ptr<model::IFigure>> &figures,
              const GridSettings &gridSettings);

  /**
   * @brief Get the current active snap point
   * @return The active snap point, or nullopt if none
   *
   * Returns the snap point that is currently highlighted as the closest
   * to the mouse cursor. This can be used by drawing tools to snap
   * the cursor position.
   */
  [[nodiscard]] std::optional<SnapPoint> getActiveSnapPoint() const;

private:
  /// Reference to UIFSMAdapter (non-owning)
  UIFSMAdapter &uiFSMAdapter_;

  /// Reference to SnapManager (non-owning)
  SnapManager &snapManager_;

  /// Current active snap point (cached from last render)
  std::optional<SnapPoint> activeSnapPoint_;

  /// Maximum number of indicators to render simultaneously
  static constexpr size_t MAX_INDICATORS = 10;

  // ==========================================================================
  // Rendering Helper Methods
  // ==========================================================================

  /**
   * @brief Render a single snap indicator
   * @param indicator The indicator visual to render
   * @param drawList ImGui draw list to render to
   *
   * Renders the appropriate shape for the snap type at the indicator position.
   */
  void renderIndicator(const SnapIndicatorVisual &indicator,
                       ImDrawList *drawList);

  /**
   * @brief Render grid snap indicator (small square)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderGridIndicator(const glm::vec2 &pos, float size,
                           const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render endpoint snap indicator (filled circle)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderEndpointIndicator(const glm::vec2 &pos, float size,
                               const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render midpoint snap indicator (triangle)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderMidpointIndicator(const glm::vec2 &pos, float size,
                               const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render center snap indicator (cross/plus)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderCenterIndicator(const glm::vec2 &pos, float size,
                             const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render intersection snap indicator (X mark)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderIntersectionIndicator(const glm::vec2 &pos, float size,
                                   const glm::vec4 &color,
                                   ImDrawList *drawList);

  /**
   * @brief Render nearest snap indicator (diamond)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderNearestIndicator(const glm::vec2 &pos, float size,
                              const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render tangent snap indicator (circle with tangent line)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderTangentIndicator(const glm::vec2 &pos, float size,
                              const glm::vec4 &color, ImDrawList *drawList);

  /**
   * @brief Render perpendicular snap indicator (right angle symbol)
   * @param pos Screen position
   * @param size Size in pixels
   * @param color Color to use
   * @param drawList ImGui draw list
   */
  void renderPerpendicularIndicator(const glm::vec2 &pos, float size,
                                    const glm::vec4 &color,
                                    ImDrawList *drawList);

  /**
   * @brief Convert glm::vec4 color to ImU32
   * @param color Color in RGBA format
   * @return ImGui color value
   */
  [[nodiscard]] ImU32 toImU32(const glm::vec4 &color) const;
};

} // namespace view
