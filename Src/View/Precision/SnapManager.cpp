#include <View/Precision/SnapManager.hpp>

#include <Model/FlatFigure.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace view {

// ============================================================================
// SnapPoint Structure Implementation
// ============================================================================

SnapPoint::SnapPoint()
    : position(0.0f), mode(SnapMode::Grid), description(""), figureId(0),
      distance(0.0f) {}

SnapPoint::SnapPoint(const glm::vec3 &pos, SnapMode m, const std::string &desc,
                     uint32_t id, float dist)
    : position(pos), mode(m), description(desc), figureId(id), distance(dist) {}

// ============================================================================
// SnapManager Constructor/Destructor
// ============================================================================

SnapManager::SnapManager(UIFSMAdapter &uiFSMAdapter, GridManager &gridManager,
                         SelectionManager &selectionManager,
                         model::IModel &model)
    : uiFSMAdapter_(uiFSMAdapter), gridManager_(gridManager),
      selectionManager_(selectionManager), model_(model) {

  // Register for figure modification events via UIFSMAdapter
  figureEventCallback_ = [this]() { onFigureModified(); };
  uiFSMAdapter_.setFigureChangedCallback(figureEventCallback_);

  // Register for camera change events via UIFSMAdapter
  cameraEventCallback_ = [this]() { onCameraChanged(); };
  uiFSMAdapter_.setCameraChangedCallback(cameraEventCallback_);

  // Register for settings change events via UIFSMAdapter
  settingsEventCallback_ = [this]() { onSettingsChanged(); };
  uiFSMAdapter_.setSnapSettingsChangedCallback(settingsEventCallback_);
}

SnapManager::~SnapManager() {
  // Callbacks are automatically cleared when SnapManager is destroyed
  // std::function is RAII and will clean up automatically
  // No manual cleanup needed
}

// ============================================================================
// Query Methods (Stateless)
// ============================================================================

bool SnapManager::isSnapEnabled(SnapMode snapMode) const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();

  switch (snapMode) {
  case SnapMode::Grid:
    return settings.gridEnabled;
  case SnapMode::Endpoint:
    return settings.endpointEnabled;
  case SnapMode::Midpoint:
    return settings.midpointEnabled;
  case SnapMode::Center:
    return settings.centerEnabled;
  case SnapMode::Intersection:
    return settings.intersectionEnabled;
  case SnapMode::Nearest:
    return settings.nearestEnabled;
  case SnapMode::Tangent:
    return settings.tangentEnabled;
  case SnapMode::Perpendicular:
    return settings.perpendicularEnabled;
  default:
    return false;
  }
}

float SnapManager::getSnapTolerance() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.tolerancePixels;
}

glm::vec4 SnapManager::getIndicatorColor() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.indicatorColor;
}

bool SnapManager::showIndicators() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.showIndicators;
}

// ============================================================================
// Global Snap Enable/Disable
// ============================================================================

void SnapManager::setSnapEnabled(bool enabled) { snapEnabled_ = enabled; }

bool SnapManager::isSnapEnabled() const { return snapEnabled_; }

// ============================================================================
// Snap Calculation Methods
// ============================================================================

std::vector<SnapPoint> SnapManager::findSnapPoints(const glm::vec2 &screenPos,
                                                   const glm::vec3 &worldPos) {
  // Check if snap is globally enabled
  if (!snapEnabled_) {
    return {};
  }

  // Check if cache is valid (event-based invalidation)
  if (!snapCache_.valid) {
    // Cache invalidated by event, must recalculate
    return calculateSnapPoints(screenPos, worldPos);
  }

  // Check distance-based invalidation
  const float distance = glm::length(screenPos - snapCache_.lastScreenPos);
  if (distance < CACHE_INVALIDATION_DISTANCE) {
    // Cursor moved less than 5px, use cached snap points
    return snapCache_.snapPoints;
  }

  // Cursor moved more than 5px, recalculate snap points
  return calculateSnapPoints(screenPos, worldPos);
}

std::optional<SnapPoint>
SnapManager::getNearestSnapPoint(const glm::vec2 &screenPos,
                                 const glm::vec3 &worldPos) {
  auto allSnapPoints = findSnapPoints(screenPos, worldPos);

  if (allSnapPoints.empty()) {
    return std::nullopt;
  }

  // Find closest snap point within tolerance
  std::optional<SnapPoint> closest;
  float minDistance = std::numeric_limits<float>::max();

  for (const auto &snapPoint : allSnapPoints) {
    if (snapPoint.distance < minDistance) {
      minDistance = snapPoint.distance;
      closest = snapPoint;
    }
  }

  // Only return if within tolerance
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  if (minDistance <= settings.tolerancePixels) {
    return closest;
  }

  return std::nullopt;
}

// ============================================================================
// Snap Mode Implementations
// ============================================================================

std::optional<SnapPoint> SnapManager::snapToGrid(const glm::vec3 &worldPos) {
  // Delegate to GridManager's snapToGrid method
  glm::vec3 snappedPos = gridManager_.snapToGrid(worldPos);

  // Calculate distance in world space (simplified approach)
  const float worldDistance = glm::length(snappedPos - worldPos);

  return SnapPoint(snappedPos, SnapMode::Grid, "Grid Point", 0, worldDistance);
}

std::vector<SnapPoint>
SnapManager::snapToEndpoints(const std::vector<uint32_t> &figureIds) {
  std::vector<SnapPoint> snapPoints;

  for (uint32_t figureId : figureIds) {
    // Get figure from IModel
    auto figure = model_.getFigure(figureId);
    if (!figure) {
      continue;
    }

    // Check Triangle
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec3 points[] = {
          glm::vec3(tri->first.x, tri->first.y, tri->first.z),
          glm::vec3(tri->second.x, tri->second.y, tri->second.z),
          glm::vec3(tri->third.x, tri->third.y, tri->third.z)};

      for (const auto &point : points) {
        snapPoints.emplace_back(point, SnapMode::Endpoint, "Endpoint",
                                figureId);
      }
    }
    // Check Quad
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec3 points[] = {
          glm::vec3(quad->first.x, quad->first.y, quad->first.z),
          glm::vec3(quad->second.x, quad->second.y, quad->second.z),
          glm::vec3(quad->third.x, quad->third.y, quad->third.z),
          glm::vec3(quad->fourth.x, quad->fourth.y, quad->fourth.z)};

      for (const auto &point : points) {
        snapPoints.emplace_back(point, SnapMode::Endpoint, "Endpoint",
                                figureId);
      }
    }
    // Check Circle (center only - circles have no endpoints)
    else if (auto circle =
                 std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                     figure)) {
      // Circles don't have endpoints, skip
      continue;
    }
    // Check Ngon (center only - ngons have no endpoints)
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
      // Ngons don't have endpoints, skip
      continue;
    }
    // Check CurveBezier3
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
      const glm::vec3 points[] = {
          glm::vec3(curve->start.x, curve->start.y, curve->start.z),
          glm::vec3(curve->end.x, curve->end.y, curve->end.z)};

      for (const auto &point : points) {
        snapPoints.emplace_back(point, SnapMode::Endpoint, "Endpoint",
                                figureId);
      }
    }
    // Check CurveBezier4
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
      const glm::vec3 points[] = {
          glm::vec3(curve->start.x, curve->start.y, curve->start.z),
          glm::vec3(curve->end.x, curve->end.y, curve->end.z)};

      for (const auto &point : points) {
        snapPoints.emplace_back(point, SnapMode::Endpoint, "Endpoint",
                                figureId);
      }
    }
  }

  return snapPoints;
}

std::vector<SnapPoint>
SnapManager::snapToMidpoints(const std::vector<uint32_t> &figureIds) {
  std::vector<SnapPoint> snapPoints;

  for (uint32_t figureId : figureIds) {
    // Get figure from IModel
    auto figure = model_.getFigure(figureId);
    if (!figure) {
      continue;
    }

    // Check Triangle
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec3 p1(tri->first.x, tri->first.y, tri->first.z);
      const glm::vec3 p2(tri->second.x, tri->second.y, tri->second.z);
      const glm::vec3 p3(tri->third.x, tri->third.y, tri->third.z);

      const glm::vec3 midpoints[] = {(p1 + p2) * 0.5f, (p2 + p3) * 0.5f,
                                     (p3 + p1) * 0.5f};

      for (const auto &midpoint : midpoints) {
        snapPoints.emplace_back(midpoint, SnapMode::Midpoint, "Midpoint",
                                figureId);
      }
    }
    // Check Quad
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec3 p1(quad->first.x, quad->first.y, quad->first.z);
      const glm::vec3 p2(quad->second.x, quad->second.y, quad->second.z);
      const glm::vec3 p3(quad->third.x, quad->third.y, quad->third.z);
      const glm::vec3 p4(quad->fourth.x, quad->fourth.y, quad->fourth.z);

      const glm::vec3 midpoints[] = {(p1 + p2) * 0.5f, (p2 + p3) * 0.5f,
                                     (p3 + p4) * 0.5f, (p4 + p1) * 0.5f};

      for (const auto &midpoint : midpoints) {
        snapPoints.emplace_back(midpoint, SnapMode::Midpoint, "Midpoint",
                                figureId);
      }
    }
    // Check Circle (center is also the midpoint)
    else if (auto circle =
                 std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                     figure)) {
      const glm::vec3 center(circle->center.x, circle->center.y,
                             circle->center.z);
      snapPoints.emplace_back(center, SnapMode::Midpoint, "Center", figureId);
    }
    // Check Ngon (center is also the midpoint)
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
      const glm::vec3 center(ngon->center.x, ngon->center.y, ngon->center.z);
      snapPoints.emplace_back(center, SnapMode::Midpoint, "Center", figureId);
    }
    // Check CurveBezier3
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
      const glm::vec3 p1(curve->start.x, curve->start.y, curve->start.z);
      const glm::vec3 p2(curve->end.x, curve->end.y, curve->end.z);
      const glm::vec3 midpoint = (p1 + p2) * 0.5f;

      snapPoints.emplace_back(midpoint, SnapMode::Midpoint, "Midpoint",
                              figureId);
    }
    // Check CurveBezier4
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
      const glm::vec3 p1(curve->start.x, curve->start.y, curve->start.z);
      const glm::vec3 p2(curve->end.x, curve->end.y, curve->end.z);
      const glm::vec3 midpoint = (p1 + p2) * 0.5f;

      snapPoints.emplace_back(midpoint, SnapMode::Midpoint, "Midpoint",
                              figureId);
    }
  }

  return snapPoints;
}

std::vector<SnapPoint>
SnapManager::snapToCenters(const std::vector<uint32_t> &figureIds) {
  std::vector<SnapPoint> snapPoints;

  for (uint32_t figureId : figureIds) {
    // Get figure from IModel
    auto figure = model_.getFigure(figureId);
    if (!figure) {
      continue;
    }

    // Check Circle
    if (auto circle =
            std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
      const glm::vec3 center(circle->center.x, circle->center.y,
                             circle->center.z);
      snapPoints.emplace_back(center, SnapMode::Center, "Center", figureId);
    }
    // Check Ngon
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
      const glm::vec3 center(ngon->center.x, ngon->center.y, ngon->center.z);
      snapPoints.emplace_back(center, SnapMode::Center, "Center", figureId);
    }
    // Check Triangle (centroid)
    else if (auto tri =
                 std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
                     figure)) {
      const glm::vec3 p1(tri->first.x, tri->first.y, tri->first.z);
      const glm::vec3 p2(tri->second.x, tri->second.y, tri->second.z);
      const glm::vec3 p3(tri->third.x, tri->third.y, tri->third.z);
      const glm::vec3 centroid = (p1 + p2 + p3) / 3.0f;

      snapPoints.emplace_back(centroid, SnapMode::Center, "Centroid", figureId);
    }
    // Check Quad (centroid)
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec3 p1(quad->first.x, quad->first.y, quad->first.z);
      const glm::vec3 p2(quad->second.x, quad->second.y, quad->second.z);
      const glm::vec3 p3(quad->third.x, quad->third.y, quad->third.z);
      const glm::vec3 p4(quad->fourth.x, quad->fourth.y, quad->fourth.z);
      const glm::vec3 centroid = (p1 + p2 + p3 + p4) / 4.0f;

      snapPoints.emplace_back(centroid, SnapMode::Center, "Centroid", figureId);
    }
  }

  return snapPoints;
}

std::vector<SnapPoint>
SnapManager::snapToIntersections(const std::vector<uint32_t> &figureIds) {
  std::vector<SnapPoint> snapPoints;

  // Extract all line segments from figures
  std::vector<std::pair<glm::vec3, glm::vec3>> lineSegments;

  for (uint32_t figureId : figureIds) {
    // Get figure from IModel
    auto figure = model_.getFigure(figureId);
    if (!figure) {
      continue;
    }

    // Check Triangle
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec3 p1(tri->first.x, tri->first.y, tri->first.z);
      const glm::vec3 p2(tri->second.x, tri->second.y, tri->second.z);
      const glm::vec3 p3(tri->third.x, tri->third.y, tri->third.z);

      lineSegments.push_back({p1, p2});
      lineSegments.push_back({p2, p3});
      lineSegments.push_back({p3, p1});
    }
    // Check Quad
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec3 p1(quad->first.x, quad->first.y, quad->first.z);
      const glm::vec3 p2(quad->second.x, quad->second.y, quad->second.z);
      const glm::vec3 p3(quad->third.x, quad->third.y, quad->third.z);
      const glm::vec3 p4(quad->fourth.x, quad->fourth.y, quad->fourth.z);

      lineSegments.push_back({p1, p2});
      lineSegments.push_back({p2, p3});
      lineSegments.push_back({p3, p4});
      lineSegments.push_back({p4, p1});
    }
    // Curves don't have line segments for intersection calculation
    // (would require curve-curve intersection algorithm)
  }

  // Calculate intersections between all line segment pairs
  for (size_t i = 0; i < lineSegments.size(); ++i) {
    for (size_t j = i + 1; j < lineSegments.size(); ++j) {
      const auto &seg1 = lineSegments[i];
      const auto &seg2 = lineSegments[j];

      auto intersection =
          lineIntersection(seg1.first, seg1.second, seg2.first, seg2.second);
      if (intersection.has_value()) {
        snapPoints.emplace_back(intersection.value(), SnapMode::Intersection,
                                "Intersection", 0);
      }
    }
  }

  return snapPoints;
}

// ============================================================================
// Performance Caching
// ============================================================================

void SnapManager::invalidateCache() {
  snapCache_.valid = false;
  snapCache_.snapPoints.clear();
  snapCache_.lastScreenPos = glm::vec2(0.0f, 0.0f);
  snapCache_.cachedFigureIds.clear();
}

// ============================================================================
// Event Handler Implementations
// ============================================================================

void SnapManager::onFigureModified() {
  // Invalidate cache on any figure modification
  invalidateCache();
}

void SnapManager::onCameraChanged() {
  // Invalidate cache on camera changes (zoom, pan, orbit)
  invalidateCache();
}

void SnapManager::onSettingsChanged() {
  // Invalidate cache on settings changes (grid or snap settings)
  invalidateCache();
}

// ============================================================================
// Helper Methods
// ============================================================================

std::vector<SnapPoint>
SnapManager::calculateSnapPoints(const glm::vec2 &screenPos,
                                 const glm::vec3 &worldPos) {
  std::vector<SnapPoint> allSnapPoints;

  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();

  // Get selected figure IDs
  std::vector<uint32_t> figureIds = selectionManager_.getSelectedFigureIds();

  // Try snap modes in priority order
  // Priority: Grid → Endpoint → Midpoint → Center → Intersection → Nearest

  // 1. Grid snap
  if (settings.gridEnabled) {
    auto gridSnap = snapToGrid(worldPos);
    if (gridSnap.has_value()) {
      // Use world distance for grid snap (simplified approach)
      const float worldDist = distanceSquared(worldPos, gridSnap->position);
      // Convert to approximate pixel distance (simplified)
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(worldDist) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        gridSnap->distance = screenDist;
        allSnapPoints.push_back(gridSnap.value());
      }
    }
  }

  // 2. Endpoint snap
  if (settings.endpointEnabled && !figureIds.empty()) {
    auto endpoints = snapToEndpoints(figureIds);
    for (auto &endpoint : endpoints) {
      const float worldDist = distanceSquared(worldPos, endpoint.position);
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(worldDist) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        endpoint.distance = screenDist;
        allSnapPoints.push_back(endpoint);
      }
    }
  }

  // 3. Midpoint snap
  if (settings.midpointEnabled && !figureIds.empty()) {
    auto midpoints = snapToMidpoints(figureIds);
    for (auto &midpoint : midpoints) {
      const float worldDist = distanceSquared(worldPos, midpoint.position);
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(worldDist) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        midpoint.distance = screenDist;
        allSnapPoints.push_back(midpoint);
      }
    }
  }

  // 4. Center snap
  if (settings.centerEnabled && !figureIds.empty()) {
    auto centers = snapToCenters(figureIds);
    for (auto &center : centers) {
      const float worldDist = distanceSquared(worldPos, center.position);
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(worldDist) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        center.distance = screenDist;
        allSnapPoints.push_back(center);
      }
    }
  }

  // 5. Intersection snap
  if (settings.intersectionEnabled && !figureIds.empty()) {
    auto intersections = snapToIntersections(figureIds);
    for (auto &intersection : intersections) {
      const float worldDist = distanceSquared(worldPos, intersection.position);
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(worldDist) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        intersection.distance = screenDist;
        allSnapPoints.push_back(intersection);
      }
    }
  }

  // 6. Nearest point snap
  if (settings.nearestEnabled && !figureIds.empty()) {
    std::optional<SnapPoint> nearestPoint;
    float minDistance = std::numeric_limits<float>::max();

    for (uint32_t figureId : figureIds) {
      // Get figure from IModel
      auto figure = model_.getFigure(figureId);
      if (!figure) {
        continue;
      }

      // Extract line segments from the figure
      std::vector<std::pair<glm::vec3, glm::vec3>> lineSegments;

      // Check Triangle
      if (auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
              figure)) {
        const glm::vec3 p1(tri->first.x, tri->first.y, tri->first.z);
        const glm::vec3 p2(tri->second.x, tri->second.y, tri->second.z);
        const glm::vec3 p3(tri->third.x, tri->third.y, tri->third.z);
        lineSegments.push_back({p1, p2});
        lineSegments.push_back({p2, p3});
        lineSegments.push_back({p3, p1});
      }
      // Check Quad
      else if (auto quad =
                   std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                       figure)) {
        const glm::vec3 p1(quad->first.x, quad->first.y, quad->first.z);
        const glm::vec3 p2(quad->second.x, quad->second.y, quad->second.z);
        const glm::vec3 p3(quad->third.x, quad->third.y, quad->third.z);
        const glm::vec3 p4(quad->fourth.x, quad->fourth.y, quad->fourth.z);
        lineSegments.push_back({p1, p2});
        lineSegments.push_back({p2, p3});
        lineSegments.push_back({p3, p4});
        lineSegments.push_back({p4, p1});
      }
      // Check CurveBezier3
      else if (auto curve = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier3>>(figure)) {
        const glm::vec3 p1(curve->start.x, curve->start.y, curve->start.z);
        const glm::vec3 p2(curve->end.x, curve->end.y, curve->end.z);
        lineSegments.push_back({p1, p2});
      }
      // Check CurveBezier4
      else if (auto curve = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier4>>(figure)) {
        const glm::vec3 p1(curve->start.x, curve->start.y, curve->start.z);
        const glm::vec3 p2(curve->end.x, curve->end.y, curve->end.z);
        lineSegments.push_back({p1, p2});
      }
      // Circles and Ngons don't have line segments for nearest point
      // calculation (would require curve-point distance algorithm)

      // For each line segment, project the query point and calculate distance
      for (const auto &segment : lineSegments) {
        glm::vec3 projected =
            projectPointOnLine(worldPos, segment.first, segment.second);
        float dist = distanceSquared(worldPos, projected);

        // Update nearest if this is closer
        if (dist < minDistance) {
          minDistance = dist;
          nearestPoint = SnapPoint(projected, SnapMode::Nearest,
                                   "Nearest point", figureId);
        }
      }
    }

    // Add nearest point if found and within tolerance
    if (nearestPoint.has_value()) {
      constexpr float WORLD_TO_PIXEL_SCALE = 10.0f;
      float screenDist = std::sqrt(minDistance) * WORLD_TO_PIXEL_SCALE;

      if (screenDist <= settings.tolerancePixels) {
        nearestPoint->distance = screenDist;
        allSnapPoints.push_back(nearestPoint.value());
      }
    }
  }

  // Sort snap points by distance (closest first)
  std::sort(allSnapPoints.begin(), allSnapPoints.end(),
            [](const SnapPoint &a, const SnapPoint &b) {
              return a.distance < b.distance;
            });

  // Update cache
  snapCache_.snapPoints = allSnapPoints;
  snapCache_.lastScreenPos = screenPos;
  snapCache_.cachedFigureIds = figureIds;
  snapCache_.valid = true;

  return allSnapPoints;
}

float SnapManager::distanceSquared(const glm::vec3 &a, const glm::vec3 &b) {
  const float dx = a.x - b.x;
  const float dy = a.y - b.y;
  const float dz = a.z - b.z;
  return dx * dx + dy * dy + dz * dz;
}

glm::vec3 SnapManager::projectPointOnLine(const glm::vec3 &point,
                                          const glm::vec3 &lineStart,
                                          const glm::vec3 &lineEnd) {
  glm::vec3 lineVec = lineEnd - lineStart;
  glm::vec3 pointVec = point - lineStart;

  float lineLengthSquared = glm::dot(lineVec, lineVec);

  // Handle degenerate case (zero-length line)
  if (lineLengthSquared < std::numeric_limits<float>::epsilon()) {
    return lineStart;
  }

  // Calculate projection parameter t
  float t = glm::dot(pointVec, lineVec) / lineLengthSquared;

  // Clamp t to [0, 1] to project onto line segment (not infinite line)
  t = std::max(0.0f, std::min(1.0f, t));

  // Calculate projected point
  return lineStart + lineVec * t;
}

std::optional<glm::vec3> SnapManager::lineIntersection(const glm::vec3 &p1,
                                                       const glm::vec3 &p2,
                                                       const glm::vec3 &p3,
                                                       const glm::vec3 &p4) {
  // Calculate intersection of two line segments in 2D (ignoring Z)
  // Using parametric line intersection formula

  float x1 = p1.x, y1 = p1.y;
  float x2 = p2.x, y2 = p2.y;
  float x3 = p3.x, y3 = p3.y;
  float x4 = p4.x, y4 = p4.y;

  // Calculate denominator
  float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  // Check if lines are parallel (denominator is zero)
  if (std::abs(denom) < std::numeric_limits<float>::epsilon()) {
    return std::nullopt;
  }

  // Calculate intersection point
  float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
  float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

  // Check if intersection is within both line segments
  if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
    float x = x1 + t * (x2 - x1);
    float y = y1 + t * (y2 - y1);

    // Use average Z from both segments
    float z = (p1.z + p2.z + p3.z + p4.z) / 4.0f;

    return glm::vec3(x, y, z);
  }

  return std::nullopt;
}

} // namespace view
