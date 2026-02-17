#include <Model/IModel.hpp>
#include <View/Precision/SnapIndicatorOverlay.hpp>
#include <algorithm>
#include <cmath>

namespace view {

// ==========================================================================
// SnapIndicatorVisual Implementation
// ==========================================================================

glm::vec4 SnapIndicatorVisual::getColor(const glm::vec4 &baseColor) const {
  /// Active indicators are slightly brighter
  if (isActive) {
    return glm::vec4(baseColor.r * 1.2f, baseColor.g * 1.2f, baseColor.b * 1.2f,
                     baseColor.a);
  }
  return baseColor;
}

// ==========================================================================
// SnapIndicatorOverlay Implementation
// ==========================================================================

SnapIndicatorOverlay::SnapIndicatorOverlay(UIFSMAdapter &uiFSMAdapter,
                                           SnapManager &snapManager)
    : uiFSMAdapter_(uiFSMAdapter), snapManager_(snapManager),
      activeSnapPoint_(std::nullopt) {}

void SnapIndicatorOverlay::render(
    const glm::vec2 &mousePosition, const glm::vec2 &canvasPosition,
    const glm::vec2 &canvasSize,
    const std::vector<std::shared_ptr<model::IFigure>> &figures,
    const GridSettings &gridSettings) {
  /// Check if snap indicators should be shown
  if (!snapManager_.showSnapIndicators()) {
    activeSnapPoint_ = std::nullopt;
    return;
  }

  /// Get the foreground draw list for rendering over the canvas
  ImDrawList *drawList = ImGui::GetForegroundDrawList();
  if (!drawList) {
    return;
  }

  /// Get snap settings
  SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  glm::vec4 indicatorColor = snapManager_.getSnapIndicatorColor();

  /// Find the active snap point
  std::optional<SnapPoint> snapPoint =
      snapManager_.findSnapPoint(mousePosition, figures);

  /// Also check for grid snap
  std::optional<SnapPoint> gridSnapPoint =
      snapManager_.findGridSnapPoint(mousePosition, gridSettings);

  /// Collect all snap points to render
  std::vector<SnapIndicatorVisual> indicators;

  /// Add grid snap point if found
  if (gridSnapPoint.has_value() && settings.gridEnabled) {
    SnapIndicatorVisual indicator;
    indicator.screenPosition = gridSnapPoint->position;
    indicator.snapMode = SnapMode::Grid;
    indicator.isActive = !snapPoint.has_value();
    indicator.size = 6.0f;
    indicators.push_back(indicator);
  }

  /// Add the primary snap point if found
  if (snapPoint.has_value()) {
    SnapIndicatorVisual indicator;
    indicator.screenPosition = snapPoint->position;
    indicator.snapMode = snapPoint->mode;
    indicator.isActive = true;
    indicator.size = 8.0f;
    indicators.push_back(indicator);

    /// Store as active snap point
    activeSnapPoint_ = snapPoint;
  } else {
    activeSnapPoint_ = std::nullopt;
  }

  /// Limit the number of indicators to render
  if (indicators.size() > MAX_INDICATORS) {
    indicators.resize(MAX_INDICATORS);
  }

  /// Render each indicator
  for (const auto &indicator : indicators) {
    /// Convert screen position to canvas-relative position
    glm::vec2 renderPos = indicator.screenPosition + canvasPosition;

    /// Check if indicator is within canvas bounds
    if (renderPos.x >= canvasPosition.x &&
        renderPos.x <= canvasPosition.x + canvasSize.x &&
        renderPos.y >= canvasPosition.y &&
        renderPos.y <= canvasPosition.y + canvasSize.y) {
      renderIndicator(indicator, drawList);
    }
  }
}

std::optional<SnapPoint> SnapIndicatorOverlay::getActiveSnapPoint() const {
  return activeSnapPoint_;
}

void SnapIndicatorOverlay::renderIndicator(const SnapIndicatorVisual &indicator,
                                           ImDrawList *drawList) {
  /// Get the base color from settings
  glm::vec4 baseColor = snapManager_.getSnapIndicatorColor();
  glm::vec4 color = indicator.getColor(baseColor);
  ImU32 imColor = toImU32(color);

  /// Render different shapes based on snap mode
  switch (indicator.snapMode) {
  case SnapMode::Grid:
    renderGridIndicator(indicator.screenPosition, indicator.size, color,
                        drawList);
    break;
  case SnapMode::Endpoint:
    renderEndpointIndicator(indicator.screenPosition, indicator.size, color,
                            drawList);
    break;
  case SnapMode::Midpoint:
    renderMidpointIndicator(indicator.screenPosition, indicator.size, color,
                            drawList);
    break;
  case SnapMode::Center:
    renderCenterIndicator(indicator.screenPosition, indicator.size, color,
                          drawList);
    break;
  case SnapMode::Intersection:
    renderIntersectionIndicator(indicator.screenPosition, indicator.size, color,
                                drawList);
    break;
  case SnapMode::Nearest:
    renderNearestIndicator(indicator.screenPosition, indicator.size, color,
                           drawList);
    break;
  case SnapMode::Tangent:
    renderTangentIndicator(indicator.screenPosition, indicator.size, color,
                           drawList);
    break;
  case SnapMode::Perpendicular:
    renderPerpendicularIndicator(indicator.screenPosition, indicator.size,
                                 color, drawList);
    break;
  default:
    /// Default to endpoint indicator for unknown types
    renderEndpointIndicator(indicator.screenPosition, indicator.size, color,
                            drawList);
    break;
  }
}

void SnapIndicatorOverlay::renderGridIndicator(const glm::vec2 &pos, float size,
                                               const glm::vec4 &color,
                                               ImDrawList *drawList) {
  /// Grid snap: small square
  ImU32 imColor = toImU32(color);
  float halfSize = size * 0.5f;
  ImVec2 p1(pos.x - halfSize, pos.y - halfSize);
  ImVec2 p2(pos.x + halfSize, pos.y + halfSize);
  drawList->AddRect(p1, p2, imColor, 0.0f, 0, 1.5f);
}

void SnapIndicatorOverlay::renderEndpointIndicator(const glm::vec2 &pos,
                                                   float size,
                                                   const glm::vec4 &color,
                                                   ImDrawList *drawList) {
  /// Endpoint snap: filled circle
  ImU32 imColor = toImU32(color);
  ImVec2 center(pos.x, pos.y);
  drawList->AddCircleFilled(center, size, imColor, 12);
}

void SnapIndicatorOverlay::renderMidpointIndicator(const glm::vec2 &pos,
                                                   float size,
                                                   const glm::vec4 &color,
                                                   ImDrawList *drawList) {
  /// Midpoint snap: triangle
  ImU32 imColor = toImU32(color);
  float height = size * 1.5f;
  float halfBase = size * 0.866f; // sqrt(3)/2

  ImVec2 p1(pos.x, pos.y - height * 0.66f);            // Top
  ImVec2 p2(pos.x - halfBase, pos.y + height * 0.33f); // Bottom left
  ImVec2 p3(pos.x + halfBase, pos.y + height * 0.33f); // Bottom right

  drawList->AddTriangle(p1, p2, p3, imColor, 1.5f);
}

void SnapIndicatorOverlay::renderCenterIndicator(const glm::vec2 &pos,
                                                 float size,
                                                 const glm::vec4 &color,
                                                 ImDrawList *drawList) {
  /// Center snap: cross/plus
  ImU32 imColor = toImU32(color);
  float halfSize = size * 0.7f;

  /// Horizontal line
  drawList->AddLine(ImVec2(pos.x - halfSize, pos.y),
                    ImVec2(pos.x + halfSize, pos.y), imColor, 1.5f);
  /// Vertical line
  drawList->AddLine(ImVec2(pos.x, pos.y - halfSize),
                    ImVec2(pos.x, pos.y + halfSize), imColor, 1.5f);
}

void SnapIndicatorOverlay::renderIntersectionIndicator(const glm::vec2 &pos,
                                                       float size,
                                                       const glm::vec4 &color,
                                                       ImDrawList *drawList) {
  /// Intersection snap: X mark
  ImU32 imColor = toImU32(color);
  float halfSize = size * 0.7f;

  /// Diagonal line 1
  drawList->AddLine(ImVec2(pos.x - halfSize, pos.y - halfSize),
                    ImVec2(pos.x + halfSize, pos.y + halfSize), imColor, 1.5f);
  /// Diagonal line 2
  drawList->AddLine(ImVec2(pos.x + halfSize, pos.y - halfSize),
                    ImVec2(pos.x - halfSize, pos.y + halfSize), imColor, 1.5f);
}

void SnapIndicatorOverlay::renderNearestIndicator(const glm::vec2 &pos,
                                                  float size,
                                                  const glm::vec4 &color,
                                                  ImDrawList *drawList) {
  /// Nearest snap: diamond
  ImU32 imColor = toImU32(color);
  float halfSize = size * 0.7f;

  ImVec2 p1(pos.x, pos.y - halfSize); // Top
  ImVec2 p2(pos.x + halfSize, pos.y); // Right
  ImVec2 p3(pos.x, pos.y + halfSize); // Bottom
  ImVec2 p4(pos.x - halfSize, pos.y); // Left

  drawList->AddQuad(p1, p2, p3, p4, imColor, 1.5f);
}

void SnapIndicatorOverlay::renderTangentIndicator(const glm::vec2 &pos,
                                                  float size,
                                                  const glm::vec4 &color,
                                                  ImDrawList *drawList) {
  /// Tangent snap: circle with tangent line
  ImU32 imColor = toImU32(color);
  ImVec2 center(pos.x, pos.y);

  /// Draw circle
  drawList->AddCircle(center, size * 0.6f, imColor, 12, 1.5f);

  /// Draw tangent line (horizontal through center)
  drawList->AddLine(ImVec2(pos.x - size, pos.y), ImVec2(pos.x + size, pos.y),
                    imColor, 1.5f);
}

void SnapIndicatorOverlay::renderPerpendicularIndicator(const glm::vec2 &pos,
                                                        float size,
                                                        const glm::vec4 &color,
                                                        ImDrawList *drawList) {
  /// Perpendicular snap: right angle symbol (L shape)
  ImU32 imColor = toImU32(color);
  float halfSize = size * 0.7f;

  /// Draw L shape
  /// Vertical line going up
  drawList->AddLine(ImVec2(pos.x - halfSize, pos.y),
                    ImVec2(pos.x - halfSize, pos.y - halfSize), imColor, 1.5f);
  /// Horizontal line going right
  drawList->AddLine(ImVec2(pos.x - halfSize, pos.y), ImVec2(pos.x, pos.y),
                    imColor, 1.5f);
}

ImU32 SnapIndicatorOverlay::toImU32(const glm::vec4 &color) const {
  /// Convert glm::vec4 (0-1 range) to ImU32 (0-255 range)
  return ImGui::ColorConvertFloat4ToU32(
      ImVec4(color.r, color.g, color.b, color.a));
}

} // namespace view
