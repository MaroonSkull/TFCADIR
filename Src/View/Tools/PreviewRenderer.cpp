/**
 * @file PreviewRenderer.cpp
 * @brief Implementation of PreviewRenderer for Phase 2 drawing tools
 *
 * This file implements the PreviewRenderer class which provides a stateless
 * rendering system for visualizing the current state of tool operations.
 *
 * Preview rendering integrates with actor system:
 * - Queries FSM for current drawing state
 * - Renders preview geometry to ImDrawList
 * - Actual figure creation handled by actors in FSM callbacks
 */

#include "PreviewRenderer.hpp"
#include "CameraController.hpp"
#include "UIFSMAdapter.hpp"
#include <cmath>

namespace view {

PreviewRenderer::PreviewRenderer(UIFSMAdapter &adapter) : adapter_(adapter) {}

glm::vec2 PreviewRenderer::worldToScreen(
    const glm::vec3 &worldPos, const CameraController &cameraController,
    const glm::mat4 &projection, const CanvasOffset &canvasOffset) const {
  /// Get the view matrix from the camera controller state
  const auto &cameraState = cameraController.getCurrentState();
  glm::mat4 view =
      glm::lookAt(cameraState.position, cameraState.target, cameraState.up);

  /// Transform world position to clip space: clip = proj * view * world
  glm::vec4 clipPos = projection * view * glm::vec4(worldPos, 1.0f);

  /// Perspective divide to get normalized device coordinates
  if (clipPos.w <= 0.0f) {
    /// Point is behind the camera, return off-screen coordinates
    return glm::vec2(-1000.0f, -1000.0f);
  }

  glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;

  /// Convert from NDC [-1, 1] to screen coordinates [0, screenWidth]
  /// Note: Actual screen size would be obtained from the viewport
  /// For now, we return normalized coordinates and add canvas offset
  glm::vec2 screenPos;
  screenPos.x = (ndc.x + 1.0f) * 0.5f + canvasOffset.x;
  screenPos.y = (1.0f - ndc.y) * 0.5f + canvasOffset.y; /// Flip Y for screen

  return screenPos;
}

void PreviewRenderer::renderPreview(const CameraController &cameraController,
                                    const glm::mat4 &projection,
                                    const CanvasOffset &canvasOffset) const {
  /// Query the FSM for the current active tool
  std::string activeTool = adapter_.getActiveTool();

  /// If no tool is active, don't render anything
  if (activeTool.empty()) {
    return;
  }

  /// Query the FSM for collected points
  std::vector<glm::vec3> collectedPoints = adapter_.getCollectedPoints();

  /// If no points collected, don't render preview
  if (collectedPoints.empty()) {
    return;
  }

  /// Render appropriate preview based on tool type
  if (activeTool == "Line3D" || activeTool == "LineInSketch") {
    renderLinePreview(collectedPoints, cameraController, projection,
                      canvasOffset);
  } else if (activeTool == "Circle3D" || activeTool == "CircleInSketch") {
    renderCirclePreview(collectedPoints, cameraController, projection,
                        canvasOffset);
  } else if (activeTool == "Arc3D") {
    renderArcPreview(collectedPoints, cameraController, projection,
                     canvasOffset);
  } else if (activeTool == "Rectangle3D") {
    renderRectanglePreview(collectedPoints, cameraController, projection,
                           canvasOffset);
  } else if (activeTool == "Polygon3D") {
    renderPolygonPreview(collectedPoints, cameraController, projection,
                         canvasOffset, true);
  } else if (activeTool == "NGon3D") {
    renderPolygonPreview(collectedPoints, cameraController, projection,
                         canvasOffset, true);
  }
}

void PreviewRenderer::renderLinePreview(
    const std::vector<glm::vec3> &points,
    const CameraController &cameraController, const glm::mat4 &projection,
    const CanvasOffset &canvasOffset) const {
  if (points.empty()) {
    return;
  }

  /// Render line from first collected point to current cursor position
  /// The actual cursor position would be obtained from ImGui::GetMousePos()
  /// For now, we'll draw lines between collected points
  if (points.size() >= 1) {
    glm::vec2 start =
        worldToScreen(points[0], cameraController, projection, canvasOffset);
    /// If we have a second point, draw to it; otherwise preview would end at
    /// cursor (not implemented here as we need ImGui mouse position)
    if (points.size() >= 2) {
      glm::vec2 end =
          worldToScreen(points[1], cameraController, projection, canvasOffset);
      /// Draw line using ImDrawList (would be obtained from
      /// ImGui::GetForegroundDrawList()) For now, this is a placeholder for the
      /// actual rendering call
      /// ImGui::GetForegroundDrawList()->AddLine(ImVec2(start.x, start.y),
      ///                                         ImVec2(end.x, end.y),
      ///                                         IM_COL32(255, 255, 255, 255),
      ///                                         2.0f);
    }
  }
}

void PreviewRenderer::renderCirclePreview(
    const std::vector<glm::vec3> &points,
    const CameraController &cameraController, const glm::mat4 &projection,
    const CanvasOffset &canvasOffset) const {
  if (points.size() < 1) {
    return;
  }

  /// First point is the circle center
  glm::vec2 center =
      worldToScreen(points[0], cameraController, projection, canvasOffset);

  /// If we have a second point, calculate radius from distance to center
  float radius = 50.0f; /// Default preview radius
  if (points.size() >= 2) {
    glm::vec2 edge =
        worldToScreen(points[1], cameraController, projection, canvasOffset);
    float dx = edge.x - center.x;
    float dy = edge.y - center.y;
    radius = std::sqrt(dx * dx + dy * dy);
  }

  /// Draw circle using ImDrawList (placeholder)
  /// ImGui::GetForegroundDrawList()->AddCircle(ImVec2(center.x, center.y),
  ///                                           radius, IM_COL32(255, 255, 255,
  ///                                           255), 32, 2.0f);
}

void PreviewRenderer::renderArcPreview(const std::vector<glm::vec3> &points,
                                       const CameraController &cameraController,
                                       const glm::mat4 &projection,
                                       const CanvasOffset &canvasOffset) const {
  if (points.size() < 1) {
    return;
  }

  /// Arc preview needs start, control, and end points
  /// Start point
  glm::vec2 start =
      worldToScreen(points[0], cameraController, projection, canvasOffset);

  /// If we have a control point, render curve preview
  if (points.size() >= 2) {
    glm::vec2 control =
        worldToScreen(points[1], cameraController, projection, canvasOffset);

    /// Draw quadratic Bezier curve preview (placeholder)
    /// ImGui::GetForegroundDrawList()->AddBezierQuadratic(
    ///     ImVec2(start.x, start.y),
    ///     ImVec2(control.x, control.y),
    ///     ImVec2(end.x, end.y),
    ///     IM_COL32(255, 255, 255, 255), 2.0f);
  }
}

void PreviewRenderer::renderRectanglePreview(
    const std::vector<glm::vec3> &points,
    const CameraController &cameraController, const glm::mat4 &projection,
    const CanvasOffset &canvasOffset) const {
  if (points.size() < 1) {
    return;
  }

  /// First point is one corner of the rectangle
  glm::vec2 corner =
      worldToScreen(points[0], cameraController, projection, canvasOffset);

  /// If we have a second point, it's the opposite corner
  if (points.size() >= 2) {
    glm::vec2 opposite =
        worldToScreen(points[1], cameraController, projection, canvasOffset);

    /// Draw rectangle (placeholder)
    /// ImGui::GetForegroundDrawList()->AddRect(
    ///     ImVec2(corner.x, corner.y),
    ///     ImVec2(opposite.x, opposite.y),
    ///     IM_COL32(255, 255, 255, 255), 0.0f, ImDrawFlags_None, 2.0f);
  }
}

void PreviewRenderer::renderPolygonPreview(
    const std::vector<glm::vec3> &points,
    const CameraController &cameraController, const glm::mat4 &projection,
    const CanvasOffset &canvasOffset, bool closed) const {
  if (points.size() < 2) {
    return;
  }

  /// Convert all collected points to screen space
  std::vector<glm::vec2> screenPoints;
  screenPoints.reserve(points.size());
  for (const auto &worldPos : points) {
    screenPoints.push_back(
        worldToScreen(worldPos, cameraController, projection, canvasOffset));
  }

  /// Draw polygon path using ImDrawList (placeholder)
  /// For closed polygons, connect last point back to first
  /// if (closed) {
  ///   ImGui::GetForegroundDrawList()->AddPolyline(
  ///       screenPoints.data(), screenPoints.size(),
  ///       IM_COL32(255, 255, 255, 255), ImDrawFlags_Closed, 2.0f);
  /// } else {
  ///   ImGui::GetForegroundDrawList()->AddPolyline(
  ///       screenPoints.data(), screenPoints.size(),
  ///       IM_COL32(255, 255, 255, 255), ImDrawFlags_None, 2.0f);
  /// }
}

} // namespace view
