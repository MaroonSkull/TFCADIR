#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace view {

// Forward declarations to avoid including headers in this header
class CameraController;
class UIFSMAdapter;

/**
 * @brief Renders preview visualization for active drawing tools
 *
 * PreviewRenderer provides a stateless rendering system for visualizing
 * the current state of tool operations. It queries the FSM for the current
 * tool and collected points, then renders appropriate preview geometry.
 *
 * All rendering is done by querying the FSM each frame, with no internal
 * state stored in this class. This ensures consistency with the Phase 1
 * architectural pattern where the FSM is the single source of truth.
 */
class PreviewRenderer {
public:
  /**
   * @brief Canvas offset for screen-space calculations
   */
  struct CanvasOffset {
    float x{0.0f};
    float y{0.0f};
  };

  /**
   * @brief Construct a PreviewRenderer
   * @param adapter Reference to the UIFSMAdapter for FSM queries
   */
  explicit PreviewRenderer(UIFSMAdapter &adapter);

  /**
   * @brief Destructor
   */
  ~PreviewRenderer() = default;

  // Copy prohibition
  PreviewRenderer(const PreviewRenderer &) = delete;
  PreviewRenderer &operator=(const PreviewRenderer &) = delete;

  // Move permission
  PreviewRenderer(PreviewRenderer &&) noexcept = default;
  PreviewRenderer &operator=(PreviewRenderer &&) noexcept = default;

  /**
   * @brief Convert world coordinates to screen coordinates
   * @param worldPos The world-space position to convert
   * @param cameraController The camera controller for view matrix
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset for screen positioning
   * @return Screen-space coordinates (x, y) in pixels
   *
   * Performs the full world-to-screen transformation:
   * 1. Apply view matrix (camera)
   * 2. Apply projection matrix
   * 3. Apply perspective divide
   * 4. Convert to screen coordinates with canvas offset
   */
  glm::vec2 worldToScreen(const glm::vec3 &worldPos,
                          const CameraController &cameraController,
                          const glm::mat4 &projection,
                          const CanvasOffset &canvasOffset) const;

  /**
   * @brief Render preview for the current tool state
   * @param cameraController The camera controller for view matrix
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset for screen positioning
   *
   * Queries the FSM for:
   * - The currently active tool (via getActiveTool())
   * - Collected points (via getCollectedPoints())
   *
   * Renders appropriate preview geometry based on the tool type:
   * - Line3D: Line from first point to current cursor position
   * - Circle3D: Circle with center and radius from two points
   * - Arc3D: Arc preview with start, end, and control points
   * - Rectangle3D: Rectangle from corner to cursor
   * - Polygon3D/NGon3D: Polygon preview with collected vertices
   * - LineInSketch/CircleInSketch: Sketch-mode 2D previews
   *
   * The rendering is stateless - each frame queries the FSM for current state.
   */
  void renderPreview(const CameraController &cameraController,
                     const glm::mat4 &projection,
                     const CanvasOffset &canvasOffset) const;

private:
  /// Reference to the UIFSMAdapter for FSM queries (non-owning)
  UIFSMAdapter &adapter_;

  /**
   * @brief Render line preview
   * @param points The collected points (start point)
   * @param cameraController The camera controller
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset
   */
  void renderLinePreview(const std::vector<glm::vec3> &points,
                         const CameraController &cameraController,
                         const glm::mat4 &projection,
                         const CanvasOffset &canvasOffset) const;

  /**
   * @brief Render circle preview
   * @param points The collected points (center point)
   * @param cameraController The camera controller
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset
   */
  void renderCirclePreview(const std::vector<glm::vec3> &points,
                           const CameraController &cameraController,
                           const glm::mat4 &projection,
                           const CanvasOffset &canvasOffset) const;

  /**
   * @brief Render arc preview
   * @param points The collected points (start point, control point)
   * @param cameraController The camera controller
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset
   */
  void renderArcPreview(const std::vector<glm::vec3> &points,
                        const CameraController &cameraController,
                        const glm::mat4 &projection,
                        const CanvasOffset &canvasOffset) const;

  /**
   * @brief Render rectangle preview
   * @param points The collected points (first corner)
   * @param cameraController The camera controller
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset
   */
  void renderRectanglePreview(const std::vector<glm::vec3> &points,
                              const CameraController &cameraController,
                              const glm::mat4 &projection,
                              const CanvasOffset &canvasOffset) const;

  /**
   * @brief Render polygon preview
   * @param points The collected polygon vertices
   * @param cameraController The camera controller
   * @param projection The projection matrix
   * @param canvasOffset The canvas offset
   * @param closed Whether to close the polygon (connect last to first)
   */
  void renderPolygonPreview(const std::vector<glm::vec3> &points,
                            const CameraController &cameraController,
                            const glm::mat4 &projection,
                            const CanvasOffset &canvasOffset,
                            bool closed = false) const;
};

} // namespace view
