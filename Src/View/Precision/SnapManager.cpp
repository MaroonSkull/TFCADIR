#include <View/Precision/SnapManager.hpp>

#include <Model/FlatFigure.hpp>
#include <algorithm>
// Must be defined before including <cmath> for M_PI to be available on MSVC
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>

namespace view {

SnapManager::SnapManager(UIFSMAdapter &uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter), snapSettingsDirty_(true),
      logger_(spdlog::get("TFCADIR")) {

  if (!logger_) {
    logger_ = spdlog::default_logger();
  }

  // Register callback to mark snap settings dirty when settings change
  uiFSMAdapter_.setSnapSettingsChangedCallback([this]() {
    snapSettingsDirty_ = true;
    logger_->debug("SnapManager: Snap settings marked dirty");
  });
}

// ==========================================================================
// State Query Methods (for SnapSettingsPanel)
// These methods delegate to UIFSMAdapter for state queries
// ==========================================================================

SnapSettings SnapManager::getSnapSettings() const {
  /// Return the current snap settings from local storage
  return uiFSMAdapter_.getSnapSettings();
}

bool SnapManager::isGridSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.gridEnabled;
}

bool SnapManager::isEndpointSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.endpointEnabled;
}

bool SnapManager::isMidpointSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.midpointEnabled;
}

bool SnapManager::isCenterSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.centerEnabled;
}

bool SnapManager::isIntersectionSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.intersectionEnabled;
}

bool SnapManager::isNearestSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.nearestEnabled;
}

bool SnapManager::isTangentSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.tangentEnabled;
}

bool SnapManager::isPerpendicularSnapEnabled() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.perpendicularEnabled;
}

float SnapManager::getSnapTolerance() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.tolerancePixels;
}

bool SnapManager::showSnapIndicators() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.showIndicators;
}

glm::vec4 SnapManager::getSnapIndicatorColor() const {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  return settings.indicatorColor;
}

// ==========================================================================
// Dirty Flag Mechanism (for caching snap point detection)
// ==========================================================================

bool SnapManager::isSnapSettingsDirty() const { return snapSettingsDirty_; }

void SnapManager::clearSnapSettingsDirty() { snapSettingsDirty_ = false; }

// ==========================================================================
// Snap Point Detection Methods (for drawing operations)
// ==========================================================================

std::optional<SnapPoint> SnapManager::findSnapPoint(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const SnapSettings settings = uiFSMAdapter_.getSnapSettings();
  const float tolerance = settings.tolerancePixels;

  /// Track the closest snap point found so far
  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  /// Helper lambda to update closest snap if closer
  auto updateClosest = [&](const std::optional<SnapPoint> &snap) {
    if (snap) {
      const float dist = distance(position, snap->position);
      if (dist < tolerance && dist < closestDistance) {
        closestSnap = snap;
        closestDistance = dist;
      }
    }
  };

  /// Check each enabled snap mode
  if (settings.gridEnabled) {
    const GridSettings gridSettings = uiFSMAdapter_.getGridSettings();
    updateClosest(findGridSnapPoint(position, gridSettings));
  }

  if (settings.endpointEnabled) {
    updateClosest(findEndpointSnap(position, figures));
  }

  if (settings.midpointEnabled) {
    updateClosest(findMidpointSnap(position, figures));
  }

  if (settings.centerEnabled) {
    updateClosest(findCenterSnap(position, figures));
  }

  if (settings.intersectionEnabled) {
    updateClosest(findIntersectionSnap(position, figures));
  }

  if (settings.nearestEnabled) {
    updateClosest(findNearestSnap(position, figures));
  }

  /// Note: Tangent and Perpendicular snap are not implemented in this subtask
  /// They will be implemented in a later subtask when required

  return closestSnap;
}

std::optional<SnapPoint>
SnapManager::findGridSnapPoint(const glm::vec2 &position,
                               const GridSettings &gridSettings) {
  const float spacing = gridSettings.majorSpacing;

  /// Calculate the nearest grid intersection
  const float snappedX = std::round(position.x / spacing) * spacing;
  const float snappedY = std::round(position.y / spacing) * spacing;

  SnapPoint snap;
  snap.position = glm::vec2(snappedX, snappedY);
  snap.mode = SnapMode::Grid;
  snap.description = "Grid";

  return snap;
}

// ==========================================================================
// Helper Methods for Snap Point Detection
// ==========================================================================

std::optional<SnapPoint> SnapManager::findEndpointSnap(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const float tolerance = uiFSMAdapter_.getSnapSettings().tolerancePixels;

  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  /// Helper lambda to check endpoint candidates
  auto checkEndpoint = [&](const glm::vec2 &point, const std::string &desc) {
    const float dist = distance(position, point);
    if (dist < tolerance && dist < closestDistance) {
      SnapPoint snap;
      snap.position = point;
      snap.mode = SnapMode::Endpoint;
      snap.description = "Endpoint (" + desc + ")";
      closestSnap = snap;
      closestDistance = dist;
    }
  };

  for (const auto &figure : figures) {
    /// Check Triangle endpoints
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec3 pos = figure->getPosition();
      checkEndpoint(glm::vec2(tri->first.x + pos.x, tri->first.y + pos.y),
                    "Triangle 1");
      checkEndpoint(glm::vec2(tri->second.x + pos.x, tri->second.y + pos.y),
                    "Triangle 2");
      checkEndpoint(glm::vec2(tri->third.x + pos.x, tri->third.y + pos.y),
                    "Triangle 3");
    }
    /// Check Quad endpoints
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec3 pos = figure->getPosition();
      checkEndpoint(glm::vec2(quad->first.x + pos.x, quad->first.y + pos.y),
                    "Quad 1");
      checkEndpoint(glm::vec2(quad->second.x + pos.x, quad->second.y + pos.y),
                    "Quad 2");
      checkEndpoint(glm::vec2(quad->third.x + pos.x, quad->third.y + pos.y),
                    "Quad 3");
      checkEndpoint(glm::vec2(quad->fourth.x + pos.x, quad->fourth.y + pos.y),
                    "Quad 4");
    }
    /// Check Circle endpoints (edge points at cardinal directions)
    else if (auto circle =
                 std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                     figure)) {
      const glm::vec3 pos = figure->getPosition();
      const float r = circle->radius;
      const glm::vec2 center(circle->center.x + pos.x,
                             circle->center.y + pos.y);

      /// Check cardinal points on the circle
      checkEndpoint(glm::vec2(center.x + r, center.y), "Circle Right");
      checkEndpoint(glm::vec2(center.x - r, center.y), "Circle Left");
      checkEndpoint(glm::vec2(center.x, center.y + r), "Circle Top");
      checkEndpoint(glm::vec2(center.x, center.y - r), "Circle Bottom");
    }
    /// Check Ngon endpoints
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
      const glm::vec3 pos = figure->getPosition();
      const float r = ngon->radius;
      const glm::vec2 center(ngon->center.x + pos.x, ngon->center.y + pos.y);
      const int n = static_cast<int>(ngon->n);

      /// Calculate vertices of the ngon
      for (int i = 0; i < n; ++i) {
        const float angle = 2.0f * M_PI * i / n;
        const float x = center.x + r * std::cos(angle);
        const float y = center.y + r * std::sin(angle);
        checkEndpoint(glm::vec2(x, y), "Ngon Vertex " + std::to_string(i));
      }
    }
    /// Check CurveBezier3 endpoints
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
      const glm::vec3 pos = figure->getPosition();
      checkEndpoint(glm::vec2(curve->start.x + pos.x, curve->start.y + pos.y),
                    "Bezier3 Start");
      checkEndpoint(glm::vec2(curve->end.x + pos.x, curve->end.y + pos.y),
                    "Bezier3 End");
    }
    /// Check CurveBezier4 endpoints
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
      const glm::vec3 pos = figure->getPosition();
      checkEndpoint(glm::vec2(curve->start.x + pos.x, curve->start.y + pos.y),
                    "Bezier4 Start");
      checkEndpoint(glm::vec2(curve->end.x + pos.x, curve->end.y + pos.y),
                    "Bezier4 End");
    }
  }

  return closestSnap;
}

std::optional<SnapPoint> SnapManager::findMidpointSnap(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const float tolerance = uiFSMAdapter_.getSnapSettings().tolerancePixels;

  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  /// Helper lambda to check midpoint candidates
  auto checkMidpoint = [&](const glm::vec2 &midpoint, const std::string &desc) {
    const float dist = distance(position, midpoint);
    if (dist < tolerance && dist < closestDistance) {
      SnapPoint snap;
      snap.position = midpoint;
      snap.mode = SnapMode::Midpoint;
      snap.description = "Midpoint (" + desc + ")";
      closestSnap = snap;
      closestDistance = dist;
    }
  };

  for (const auto &figure : figures) {
    const glm::vec3 pos = figure->getPosition();

    /// Check Triangle midpoints
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec2 p1(tri->first.x + pos.x, tri->first.y + pos.y);
      const glm::vec2 p2(tri->second.x + pos.x, tri->second.y + pos.y);
      const glm::vec2 p3(tri->third.x + pos.x, tri->third.y + pos.y);

      checkMidpoint(glm::vec2((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f),
                    "Triangle 1-2");
      checkMidpoint(glm::vec2((p2.x + p3.x) / 2.0f, (p2.y + p3.y) / 2.0f),
                    "Triangle 2-3");
      checkMidpoint(glm::vec2((p3.x + p1.x) / 2.0f, (p3.y + p1.y) / 2.0f),
                    "Triangle 3-1");
    }
    /// Check Quad midpoints
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec2 p1(quad->first.x + pos.x, quad->first.y + pos.y);
      const glm::vec2 p2(quad->second.x + pos.x, quad->second.y + pos.y);
      const glm::vec2 p3(quad->third.x + pos.x, quad->third.y + pos.y);
      const glm::vec2 p4(quad->fourth.x + pos.x, quad->fourth.y + pos.y);

      checkMidpoint(glm::vec2((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f),
                    "Quad 1-2");
      checkMidpoint(glm::vec2((p2.x + p3.x) / 2.0f, (p2.y + p3.y) / 2.0f),
                    "Quad 2-3");
      checkMidpoint(glm::vec2((p3.x + p4.x) / 2.0f, (p3.y + p4.y) / 2.0f),
                    "Quad 3-4");
      checkMidpoint(glm::vec2((p4.x + p1.x) / 2.0f, (p4.y + p1.y) / 2.0f),
                    "Quad 4-1");
    }
    /// Check Curve midpoints
    else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
      const glm::vec2 p1(curve->start.x + pos.x, curve->start.y + pos.y);
      const glm::vec2 p2(curve->end.x + pos.x, curve->end.y + pos.y);
      checkMidpoint(glm::vec2((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f),
                    "Bezier3");
    } else if (auto curve = std::dynamic_pointer_cast<
                   model::Figure<model::CurveBezier4>>(figure)) {
      const glm::vec2 p1(curve->start.x + pos.x, curve->start.y + pos.y);
      const glm::vec2 p2(curve->end.x + pos.x, curve->end.y + pos.y);
      checkMidpoint(glm::vec2((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f),
                    "Bezier4");
    }
  }

  return closestSnap;
}

std::optional<SnapPoint> SnapManager::findCenterSnap(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const float tolerance = uiFSMAdapter_.getSnapSettings().tolerancePixels;

  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  /// Helper lambda to check center candidates
  auto checkCenter = [&](const glm::vec2 &center, const std::string &desc) {
    const float dist = distance(position, center);
    if (dist < tolerance && dist < closestDistance) {
      SnapPoint snap;
      snap.position = center;
      snap.mode = SnapMode::Center;
      snap.description = "Center (" + desc + ")";
      closestSnap = snap;
      closestDistance = dist;
    }
  };

  for (const auto &figure : figures) {
    const glm::vec3 pos = figure->getPosition();

    /// Check Circle center
    if (auto circle =
            std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
      const glm::vec2 center(circle->center.x + pos.x,
                             circle->center.y + pos.y);
      checkCenter(center, "Circle");
    }
    /// Check Ngon center
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
      const glm::vec2 center(ngon->center.x + pos.x, ngon->center.y + pos.y);
      checkCenter(center, "Ngon");
    }
    /// Check Triangle centroid
    else if (auto tri =
                 std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
                     figure)) {
      const glm::vec2 p1(tri->first.x + pos.x, tri->first.y + pos.y);
      const glm::vec2 p2(tri->second.x + pos.x, tri->second.y + pos.y);
      const glm::vec2 p3(tri->third.x + pos.x, tri->third.y + pos.y);
      const glm::vec2 centroid((p1.x + p2.x + p3.x) / 3.0f,
                               (p1.y + p2.y + p3.y) / 3.0f);
      checkCenter(centroid, "Triangle");
    }
    /// Check Quad center
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec2 p1(quad->first.x + pos.x, quad->first.y + pos.y);
      const glm::vec2 p2(quad->second.x + pos.x, quad->second.y + pos.y);
      const glm::vec2 p3(quad->third.x + pos.x, quad->third.y + pos.y);
      const glm::vec2 p4(quad->fourth.x + pos.x, quad->fourth.y + pos.y);
      const glm::vec2 center((p1.x + p2.x + p3.x + p4.x) / 4.0f,
                             (p1.y + p2.y + p3.y + p4.y) / 4.0f);
      checkCenter(center, "Quad");
    }
  }

  return closestSnap;
}

std::optional<SnapPoint> SnapManager::findIntersectionSnap(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const float tolerance = uiFSMAdapter_.getSnapSettings().tolerancePixels;

  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  /// Helper lambda to check intersection candidates
  auto checkIntersection = [&](const glm::vec2 &intersection,
                               const std::string &desc) {
    const float dist = distance(position, intersection);
    if (dist < tolerance && dist < closestDistance) {
      SnapPoint snap;
      snap.position = intersection;
      snap.mode = SnapMode::Intersection;
      snap.description = "Intersection (" + desc + ")";
      closestSnap = snap;
      closestDistance = dist;
    }
  };

  /// Collect all line segments from figures
  std::vector<std::pair<glm::vec2, glm::vec2>> segments;

  for (const auto &figure : figures) {
    const glm::vec3 pos = figure->getPosition();

    /// Extract segments from Triangle
    if (auto tri =
            std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      const glm::vec2 p1(tri->first.x + pos.x, tri->first.y + pos.y);
      const glm::vec2 p2(tri->second.x + pos.x, tri->second.y + pos.y);
      const glm::vec2 p3(tri->third.x + pos.x, tri->third.y + pos.y);
      segments.push_back({p1, p2});
      segments.push_back({p2, p3});
      segments.push_back({p3, p1});
    }
    /// Extract segments from Quad
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
      const glm::vec2 p1(quad->first.x + pos.x, quad->first.y + pos.y);
      const glm::vec2 p2(quad->second.x + pos.x, quad->second.y + pos.y);
      const glm::vec2 p3(quad->third.x + pos.x, quad->third.y + pos.y);
      const glm::vec2 p4(quad->fourth.x + pos.x, quad->fourth.y + pos.y);
      segments.push_back({p1, p2});
      segments.push_back({p2, p3});
      segments.push_back({p3, p4});
      segments.push_back({p4, p1});
    }
  }

  /// Check intersections between all pairs of segments
  for (size_t i = 0; i < segments.size(); ++i) {
    for (size_t j = i + 1; j < segments.size(); ++j) {
      const auto &seg1 = segments[i];
      const auto &seg2 = segments[j];

      /// Skip if segments share an endpoint (they're from the same figure)
      if (seg1.first == seg2.first || seg1.first == seg2.second ||
          seg1.second == seg2.first || seg1.second == seg2.second) {
        continue;
      }

      /// Calculate intersection using line-line intersection formula
      const float x1 = seg1.first.x, y1 = seg1.first.y;
      const float x2 = seg1.second.x, y2 = seg1.second.y;
      const float x3 = seg2.first.x, y3 = seg2.first.y;
      const float x4 = seg2.second.x, y4 = seg2.second.y;

      const float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

      /// Lines are parallel if denominator is zero
      if (std::abs(denom) < 1e-6f) {
        continue;
      }

      const float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
      const float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

      /// Check if intersection is within both segments
      if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        const glm::vec2 intersection(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
        checkIntersection(intersection, "Seg " + std::to_string(i) + " & " +
                                            std::to_string(j));
      }
    }
  }

  return closestSnap;
}

std::optional<SnapPoint> SnapManager::findNearestSnap(
    const glm::vec2 &position,
    const std::vector<std::shared_ptr<model::IFigure>> &figures) {
  const float tolerance = uiFSMAdapter_.getSnapSettings().tolerancePixels;

  std::optional<SnapPoint> closestSnap;
  float closestDistance = std::numeric_limits<float>::max();

  for (const auto &figure : figures) {
    const glm::vec3 pos = figure->getPosition();

    /// For Circle and Ngon, find nearest point on the perimeter
    if (auto circle =
            std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
      const glm::vec2 center(circle->center.x + pos.x,
                             circle->center.y + pos.y);
      const float r = circle->radius;

      /// Calculate direction from center to position
      const glm::vec2 dir = position - center;
      const float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

      if (dist > 1e-6f) {
        /// Nearest point on circle is along the direction vector
        const glm::vec2 nearest = center + (dir / dist) * r;
        const float snapDist = distance(position, nearest);

        if (snapDist < tolerance && snapDist < closestDistance) {
          SnapPoint snap;
          snap.position = nearest;
          snap.mode = SnapMode::Nearest;
          snap.description = "Nearest (Circle)";
          closestSnap = snap;
          closestDistance = snapDist;
        }
      }
    } else if (auto ngon =
                   std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                       figure)) {
      const glm::vec2 center(ngon->center.x + pos.x, ngon->center.y + pos.y);
      const float r = ngon->radius;
      const int n = static_cast<int>(ngon->n);

      /// Find nearest point on ngon perimeter
      for (int i = 0; i < n; ++i) {
        const float angle1 = 2.0f * M_PI * i / n;
        const float angle2 = 2.0f * M_PI * (i + 1) / n;

        const glm::vec2 v1(center.x + r * std::cos(angle1),
                           center.y + r * std::sin(angle1));
        const glm::vec2 v2(center.x + r * std::cos(angle2),
                           center.y + r * std::sin(angle2));

        /// Find nearest point on this segment
        const glm::vec2 seg = v2 - v1;
        const float segLengthSq = seg.x * seg.x + seg.y * seg.y;

        if (segLengthSq > 1e-6f) {
          const glm::vec2 toPos = position - v1;
          float t = (toPos.x * seg.x + toPos.y * seg.y) / segLengthSq;
          t = std::max(0.0f, std::min(1.0f, t));

          const glm::vec2 nearest = v1 + t * seg;
          const float snapDist = distance(position, nearest);

          if (snapDist < tolerance && snapDist < closestDistance) {
            SnapPoint snap;
            snap.position = nearest;
            snap.mode = SnapMode::Nearest;
            snap.description = "Nearest (Ngon Edge " + std::to_string(i) + ")";
            closestSnap = snap;
            closestDistance = snapDist;
          }
        }
      }
    }
    /// For other figures, find nearest point on edges
    else {
      std::vector<std::pair<glm::vec2, glm::vec2>> segments;

      if (auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(
              figure)) {
        const glm::vec2 p1(tri->first.x + pos.x, tri->first.y + pos.y);
        const glm::vec2 p2(tri->second.x + pos.x, tri->second.y + pos.y);
        const glm::vec2 p3(tri->third.x + pos.x, tri->third.y + pos.y);
        segments.push_back({p1, p2});
        segments.push_back({p2, p3});
        segments.push_back({p3, p1});
      } else if (auto quad =
                     std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                         figure)) {
        const glm::vec2 p1(quad->first.x + pos.x, quad->first.y + pos.y);
        const glm::vec2 p2(quad->second.x + pos.x, quad->second.y + pos.y);
        const glm::vec2 p3(quad->third.x + pos.x, quad->third.y + pos.y);
        const glm::vec2 p4(quad->fourth.x + pos.x, quad->fourth.y + pos.y);
        segments.push_back({p1, p2});
        segments.push_back({p2, p3});
        segments.push_back({p3, p4});
        segments.push_back({p4, p1});
      }

      for (const auto &seg : segments) {
        const glm::vec2 segVec = seg.second - seg.first;
        const float segLengthSq = segVec.x * segVec.x + segVec.y * segVec.y;

        if (segLengthSq > 1e-6f) {
          const glm::vec2 toPos = position - seg.first;
          float t = (toPos.x * segVec.x + toPos.y * segVec.y) / segLengthSq;
          t = std::max(0.0f, std::min(1.0f, t));

          const glm::vec2 nearest = seg.first + t * segVec;
          const float snapDist = distance(position, nearest);

          if (snapDist < tolerance && snapDist < closestDistance) {
            SnapPoint snap;
            snap.position = nearest;
            snap.mode = SnapMode::Nearest;
            snap.description = "Nearest (Edge)";
            closestSnap = snap;
            closestDistance = snapDist;
          }
        }
      }
    }
  }

  return closestSnap;
}

float SnapManager::distance(const glm::vec2 &a, const glm::vec2 &b) const {
  const float dx = a.x - b.x;
  const float dy = a.y - b.y;
  return std::sqrt(dx * dx + dy * dy);
}

} // namespace view
