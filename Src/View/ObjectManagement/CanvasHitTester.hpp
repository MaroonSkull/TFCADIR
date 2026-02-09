#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <memory>

namespace model {
class FlatFigures;
class IFigure;
} // namespace model

namespace view {

/**
 * @brief Provides stateless hit detection algorithms for 2D figures on canvas
 * @details Implements mathematical algorithms for point-in-figure tests and
 *          box selection. All methods are stateless and rely on passed-in
 *          viewport transform matrices.
 */
class CanvasHitTester {
public:
  /**
   * @brief Constructs a new CanvasHitTester
   * @param model Reference to the figures model
   */
  explicit CanvasHitTester(model::FlatFigures& model);

  /**
   * @brief Destructor
   */
  ~CanvasHitTester() = default;

  /**
   * @brief Tests if a screen coordinate hits any 2D figure
   * @param screenX Screen X coordinate in pixels
   * @param screenY Screen Y coordinate in pixels
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @param tolerance Hit tolerance in pixels (default 5)
   * @return Figure ID of the hit figure, or 0 if no hit
   * @details Performs screen-to-world transformation and tests each figure
   *          using appropriate geometric algorithms. Returns the topmost
   *          figure (last in render order) at the hit point.
   */
  uint32_t hitTest(float screenX, float screenY, float viewportWidth,
                   float viewportHeight, const glm::mat4& modelView,
                   const glm::mat4& projection, float tolerance = 5.0f);

  /**
   * @brief Tests if a selection box intersects any 2D figures
   * @param screenMinX Selection box minimum X in pixels
   * @param screenMinY Selection box minimum Y in pixels
   * @param screenMaxX Selection box maximum X in pixels
   * @param screenMaxY Selection box maximum Y in pixels
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return Vector of figure IDs that intersect the selection box
   * @details Tests each figure for intersection with the selection box.
   *          Returns all figures that intersect or are contained within
   *          the box.
   */
  std::vector<uint32_t> hitTestBox(float screenMinX, float screenMinY,
                                    float screenMaxX, float screenMaxY,
                                    float viewportWidth, float viewportHeight,
                                    const glm::mat4& modelView,
                                    const glm::mat4& projection);

private:
  /// Reference to the figures model
  model::FlatFigures& model_;

  /**
   * @brief Transforms screen coordinates to world coordinates
   * @param screenX Screen X coordinate in pixels
   * @param screenY Screen Y coordinate in pixels
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return World position as vec3 (z = 0 for 2D)
   */
  glm::vec3 screenToWorld(float screenX, float screenY, float viewportWidth,
                          float viewportHeight, const glm::mat4& modelView,
                          const glm::mat4& projection);

  /**
   * @brief Tests if a point is inside a circle
   * @param point World position to test
   * @param center Circle center position
   * @param radius Circle radius
   * @param tolerance Hit tolerance in world units
   * @return true if the point is inside or within tolerance of the circle
   */
  bool isPointInCircle(const glm::vec2& point, const glm::vec2& center,
                       float radius, float tolerance);

  /**
   * @brief Tests if a point is inside a triangle using barycentric coordinates
   * @param point World position to test
   * @param v0 First triangle vertex
   * @param v1 Second triangle vertex
   * @param v2 Third triangle vertex
   * @param tolerance Hit tolerance in world units
   * @return true if the point is inside or within tolerance of the triangle
   */
  bool isPointInTriangle(const glm::vec2& point, const glm::vec2& v0,
                         const glm::vec2& v1, const glm::vec2& v2,
                         float tolerance);

  /**
   * @brief Tests if a point is inside a convex polygon using ray casting
   * @param point World position to test
   * @param vertices Polygon vertices in order
   * @param tolerance Hit tolerance in world units
   * @return true if the point is inside or within tolerance of the polygon
   */
  bool isPointInPolygon(const glm::vec2& point,
                        const std::vector<glm::vec2>& vertices,
                        float tolerance);

  /**
   * @brief Tests if a point is near a line segment
   * @param point World position to test
   * @param start Line segment start
   * @param end Line segment end
   * @param tolerance Hit tolerance in world units
   * @return true if the point is within tolerance of the line segment
   */
  bool isPointNearLine(const glm::vec2& point, const glm::vec2& start,
                       const glm::vec2& end, float tolerance);

  /**
   * @brief Tests if a point is near a set of points
   * @param point World position to test
   * @param points Vector of points to check against
   * @param tolerance Hit tolerance in world units
   * @return true if the point is within tolerance of any point
   */
  bool isPointNearAnyPoint(const glm::vec2& point,
                           const std::vector<glm::vec2>& points,
                           float tolerance);

  /**
   * @brief Tests if a box intersects a circle
   * @param boxMin Box minimum corner in world coordinates
   * @param boxMax Box maximum corner in world coordinates
   * @param center Circle center
   * @param radius Circle radius
   * @return true if the box intersects the circle
   */
  bool doesBoxIntersectCircle(const glm::vec2& boxMin, const glm::vec2& boxMax,
                              const glm::vec2& center, float radius);

  /**
   * @brief Tests if a box intersects a triangle
   * @param boxMin Box minimum corner in world coordinates
   * @param boxMax Box maximum corner in world coordinates
   * @param v0 First triangle vertex
   * @param v1 Second triangle vertex
   * @param v2 Third triangle vertex
   * @return true if the box intersects the triangle
   */
  bool doesBoxIntersectTriangle(const glm::vec2& boxMin,
                                const glm::vec2& boxMax, const glm::vec2& v0,
                                const glm::vec2& v1, const glm::vec2& v2);

  /**
   * @brief Tests if a box intersects a convex polygon
   * @param boxMin Box minimum corner in world coordinates
   * @param boxMax Box maximum corner in world coordinates
   * @param vertices Polygon vertices in order
   * @return true if the box intersects the polygon
   */
  bool doesBoxIntersectPolygon(const glm::vec2& boxMin,
                               const glm::vec2& boxMax,
                               const std::vector<glm::vec2>& vertices);

  /**
   * @brief Tests if two line segments intersect
   * @param p1 First segment start
   * @param p2 First segment end
   * @param p3 Second segment start
   * @param p4 Second segment end
   * @return true if the segments intersect
   */
  static bool segmentsIntersect(const glm::vec2& p1, const glm::vec2& p2,
                                const glm::vec2& p3, const glm::vec2& p4);

  /**
   * @brief Calculates the squared distance between two points
   * @param a First point
   * @param b Second point
   * @return Squared distance (avoids sqrt for performance)
   */
  static float distanceSquared(const glm::vec2& a, const glm::vec2& b);

  /// Default hit tolerance in pixels
  static constexpr float DEFAULT_TOLERANCE_PIXELS = 5.0f;
};

} // namespace view
