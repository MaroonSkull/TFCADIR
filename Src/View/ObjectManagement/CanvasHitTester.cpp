#include "CanvasHitTester.hpp"
#include <Model/FlatFigure.hpp>
#include <algorithm>
#include <cmath>
#include <memory>

namespace view {

CanvasHitTester::CanvasHitTester(model::FlatFigures& model) : model_(model) {}

uint32_t CanvasHitTester::hitTest(float screenX, float screenY,
                                   float viewportWidth, float viewportHeight,
                                   const glm::mat4& modelView,
                                   const glm::mat4& projection,
                                   float tolerance) {
  // Convert screen coordinates to world coordinates
  glm::vec3 worldPos = screenToWorld(screenX, screenY, viewportWidth,
                                      viewportHeight, modelView, projection);
  glm::vec2 point(worldPos.x, worldPos.y);

  // Calculate tolerance in world units (approximate)
  // For orthographic projection, we can estimate from viewport size
  glm::vec4 viewPort(0.0f, 0.0f, viewportWidth, viewportHeight);
  glm::vec3 corner0 = glm::unProject(glm::vec3(0.0f, 0.0f, 0.0f), modelView,
                                     projection, viewPort);
  glm::vec3 corner1 =
      glm::unProject(glm::vec3(1.0f, 0.0f, 0.0f), modelView, projection,
                     viewPort);
  float pixelToWorld = glm::distance(corner0, corner1);
  float worldTolerance = tolerance * pixelToWorld;

  // Iterate through all figures in reverse order (topmost first)
  size_t count = model_.getFigureCount();
  for (size_t i = count; i > 0; --i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i - 1));
    if (!figure) {
      continue;
    }

    bool hit = false;

    // Try casting to each figure type
    if (auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      glm::vec2 v0(tri->first.x, tri->first.y);
      glm::vec2 v1(tri->second.x, tri->second.y);
      glm::vec2 v2(tri->third.x, tri->third.y);
      hit = isPointInTriangle(point, v0, v1, v2, worldTolerance);
    }
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
      std::vector<glm::vec2> vertices = {
          glm::vec2(quad->first.x, quad->first.y),
          glm::vec2(quad->second.x, quad->second.y),
          glm::vec2(quad->third.x, quad->third.y),
          glm::vec2(quad->fourth.x, quad->fourth.y)
      };
      hit = isPointInPolygon(point, vertices, worldTolerance);
    }
    else if (auto circle = std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
      glm::vec2 center(circle->center.x, circle->center.y);
      hit = isPointInCircle(point, center, circle->radius, worldTolerance);
    }
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
      // Generate ngon vertices from center, first point, n, and radius
      std::vector<glm::vec2> vertices;
      int numSides = static_cast<int>(ngon->n);
      float angle = std::atan2(ngon->first.y - ngon->center.y,
                               ngon->first.x - ngon->center.x);
      float angleStep = 2.0f * M_PI / numSides;

      vertices.reserve(numSides);
      for (int i = 0; i < numSides; ++i) {
        float a = angle + i * angleStep;
        vertices.emplace_back(
          ngon->center.x + ngon->radius * std::cos(a),
          ngon->center.y + ngon->radius * std::sin(a)
        );
      }
      hit = isPointInPolygon(point, vertices, worldTolerance);
    }
    else if (auto curve = std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure)) {
      std::vector<glm::vec2> points = {
        glm::vec2(curve->start.x, curve->start.y),
        glm::vec2(curve->end.x, curve->end.y),
        glm::vec2(curve->first.x, curve->first.y)
      };
      hit = isPointNearAnyPoint(point, points, worldTolerance);
    }
    else if (auto curve = std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure)) {
      std::vector<glm::vec2> points = {
        glm::vec2(curve->start.x, curve->start.y),
        glm::vec2(curve->end.x, curve->end.y),
        glm::vec2(curve->first.x, curve->first.y),
        glm::vec2(curve->second.x, curve->second.y)
      };
      hit = isPointNearAnyPoint(point, points, worldTolerance);
    }

    if (hit) {
      return figure->getId();
    }
  }

  return 0; // No hit
}

std::vector<uint32_t> CanvasHitTester::hitTestBox(
    float screenMinX, float screenMinY, float screenMaxX, float screenMaxY,
    float viewportWidth, float viewportHeight, const glm::mat4& modelView,
    const glm::mat4& projection) {
  std::vector<uint32_t> hits;

  // Convert screen box to world coordinates
  glm::vec3 worldMin = screenToWorld(screenMinX, screenMinY, viewportWidth,
                                      viewportHeight, modelView, projection);
  glm::vec3 worldMax = screenToWorld(screenMaxX, screenMaxY, viewportWidth,
                                      viewportHeight, modelView, projection);

  glm::vec2 boxMin(glm::min(worldMin.x, worldMax.x),
                   glm::min(worldMin.y, worldMax.y));
  glm::vec2 boxMax(glm::max(worldMin.x, worldMax.x),
                   glm::max(worldMin.y, worldMax.y));

  // Iterate through all figures
  size_t count = model_.getFigureCount();
  for (size_t i = 0; i < count; ++i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i));
    if (!figure) {
      continue;
    }

    bool hit = false;

    // Try casting to each figure type
    if (auto tri = std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
      glm::vec2 v0(tri->first.x, tri->first.y);
      glm::vec2 v1(tri->second.x, tri->second.y);
      glm::vec2 v2(tri->third.x, tri->third.y);
      hit = doesBoxIntersectTriangle(boxMin, boxMax, v0, v1, v2);
    }
    else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
      std::vector<glm::vec2> vertices = {
          glm::vec2(quad->first.x, quad->first.y),
          glm::vec2(quad->second.x, quad->second.y),
          glm::vec2(quad->third.x, quad->third.y),
          glm::vec2(quad->fourth.x, quad->fourth.y)
      };
      hit = doesBoxIntersectPolygon(boxMin, boxMax, vertices);
    }
    else if (auto circle = std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
      glm::vec2 center(circle->center.x, circle->center.y);
      hit = doesBoxIntersectCircle(boxMin, boxMax, center, circle->radius);
    }
    else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
      // For ngon, check if any vertex is inside box or if box contains center
      glm::vec2 center(ngon->center.x, ngon->center.y);
      if (center.x >= boxMin.x && center.x <= boxMax.x &&
          center.y >= boxMin.y && center.y <= boxMax.y) {
        hit = true;
      } else {
        // Generate ngon vertices and check if any are inside the box
        int numSides = static_cast<int>(ngon->n);
        float angle = std::atan2(ngon->first.y - ngon->center.y,
                                 ngon->first.x - ngon->center.x);
        float angleStep = 2.0f * M_PI / numSides;

        for (int i = 0; i < numSides && !hit; ++i) {
          float a = angle + i * angleStep;
          glm::vec2 vertex(
            ngon->center.x + ngon->radius * std::cos(a),
            ngon->center.y + ngon->radius * std::sin(a)
          );
          if (vertex.x >= boxMin.x && vertex.x <= boxMax.x &&
              vertex.y >= boxMin.y && vertex.y <= boxMax.y) {
            hit = true;
          }
        }
      }
    }
    else if (auto curve = std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure)) {
      // Check if any control point is in the box
      if ((curve->start.x >= boxMin.x && curve->start.x <= boxMax.x &&
           curve->start.y >= boxMin.y && curve->start.y <= boxMax.y) ||
          (curve->end.x >= boxMin.x && curve->end.x <= boxMax.x &&
           curve->end.y >= boxMin.y && curve->end.y <= boxMax.y) ||
          (curve->first.x >= boxMin.x && curve->first.x <= boxMax.x &&
           curve->first.y >= boxMin.y && curve->first.y <= boxMax.y)) {
        hit = true;
      }
    }
    else if (auto curve = std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure)) {
      // Check if any control point is in the box
      if ((curve->start.x >= boxMin.x && curve->start.x <= boxMax.x &&
           curve->start.y >= boxMin.y && curve->start.y <= boxMax.y) ||
          (curve->end.x >= boxMin.x && curve->end.x <= boxMax.x &&
           curve->end.y >= boxMin.y && curve->end.y <= boxMax.y) ||
          (curve->first.x >= boxMin.x && curve->first.x <= boxMax.x &&
           curve->first.y >= boxMin.y && curve->first.y <= boxMax.y) ||
          (curve->second.x >= boxMin.x && curve->second.x <= boxMax.x &&
           curve->second.y >= boxMin.y && curve->second.y <= boxMax.y)) {
        hit = true;
      }
    }

    if (hit) {
      hits.push_back(figure->getId());
    }
  }

  return hits;
}

glm::vec3 CanvasHitTester::screenToWorld(float screenX, float screenY,
                                          float viewportWidth,
                                          float viewportHeight,
                                          const glm::mat4& modelView,
                                          const glm::mat4& projection) {
  // OpenGL uses viewport with origin at bottom-left
  // ImGui uses origin at top-left, so we need to flip Y
  float glScreenY = viewportHeight - screenY;

  glm::vec4 viewport(0.0f, 0.0f, viewportWidth, viewportHeight);
  glm::vec3 worldPos =
      glm::unProject(glm::vec3(screenX, glScreenY, 0.0f), modelView,
                     projection, viewport);

  // For 2D figures, z should be 0
  worldPos.z = 0.0f;

  return worldPos;
}

bool CanvasHitTester::isPointInCircle(const glm::vec2& point,
                                       const glm::vec2& center, float radius,
                                       float tolerance) {
  float distSquared = distanceSquared(point, center);
  float radiusWithTolerance = radius + tolerance;
  return distSquared <= radiusWithTolerance * radiusWithTolerance;
}

bool CanvasHitTester::isPointInTriangle(const glm::vec2& point,
                                         const glm::vec2& v0,
                                         const glm::vec2& v1,
                                         const glm::vec2& v2,
                                         float tolerance) {
  // Barycentric coordinate test
  glm::vec2 v0v1 = v1 - v0;
  glm::vec2 v0v2 = v2 - v0;
  glm::vec2 v0p = point - v0;

  float dot00 = glm::dot(v0v2, v0v2);
  float dot01 = glm::dot(v0v2, v0v1);
  float dot02 = glm::dot(v0v2, v0p);
  float dot11 = glm::dot(v0v1, v0v1);
  float dot12 = glm::dot(v0v1, v0p);

  float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  bool inside = (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);

  // If not inside, check if point is near any edge
  if (!inside && tolerance > 0.0f) {
    float tolSquared = tolerance * tolerance;
    if (distanceSquared(point, v0) <= tolSquared ||
        distanceSquared(point, v1) <= tolSquared ||
        distanceSquared(point, v2) <= tolSquared) {
      return true;
    }
    if (isPointNearLine(point, v0, v1, tolerance) ||
        isPointNearLine(point, v1, v2, tolerance) ||
        isPointNearLine(point, v2, v0, tolerance)) {
      return true;
    }
  }

  return inside;
}

bool CanvasHitTester::isPointInPolygon(const glm::vec2& point,
                                        const std::vector<glm::vec2>& vertices,
                                        float tolerance) {
  if (vertices.size() < 3) {
    return false;
  }

  // Ray casting algorithm for point-in-polygon test
  int intersections = 0;
  size_t n = vertices.size();

  for (size_t i = 0; i < n; ++i) {
    const glm::vec2& v1 = vertices[i];
    const glm::vec2& v2 = vertices[(i + 1) % n];

    // Check if point is on the edge (with tolerance)
    if (isPointNearLine(point, v1, v2, tolerance)) {
      return true;
    }

    // Ray casting: count intersections with ray going to the right
    if ((v1.y > point.y) != (v2.y > point.y)) {
      float xIntersection =
          (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x;
      if (point.x < xIntersection) {
        ++intersections;
      }
    }
  }

  return (intersections % 2) == 1;
}

bool CanvasHitTester::isPointNearLine(const glm::vec2& point,
                                       const glm::vec2& start,
                                       const glm::vec2& end, float tolerance) {
  float tolSquared = tolerance * tolerance;

  // Check if point is near endpoints
  if (distanceSquared(point, start) <= tolSquared ||
      distanceSquared(point, end) <= tolSquared) {
    return true;
  }

  // Check if point projects onto the line segment
  glm::vec2 line = end - start;
  float lineLengthSquared = glm::dot(line, line);
  if (lineLengthSquared < 1e-6f) {
    return false; // Degenerate line
  }

  glm::vec2 pointToStart = point - start;
  float t = glm::dot(pointToStart, line) / lineLengthSquared;

  // Clamp t to segment [0, 1]
  t = glm::clamp(t, 0.0f, 1.0f);

  glm::vec2 closestPoint = start + t * line;
  float distSquared = distanceSquared(point, closestPoint);

  return distSquared <= tolSquared;
}

bool CanvasHitTester::isPointNearAnyPoint(const glm::vec2& point,
                                           const std::vector<glm::vec2>& points,
                                           float tolerance) {
  float tolSquared = tolerance * tolerance;
  for (const auto& p : points) {
    if (distanceSquared(point, p) <= tolSquared) {
      return true;
    }
  }
  return false;
}

bool CanvasHitTester::doesBoxIntersectCircle(const glm::vec2& boxMin,
                                              const glm::vec2& boxMax,
                                              const glm::vec2& center,
                                              float radius) {
  // Find closest point on box to circle center
  glm::vec2 closest(
      glm::clamp(center.x, boxMin.x, boxMax.x),
      glm::clamp(center.y, boxMin.y, boxMax.y));

  // Check if distance from closest point to center is less than radius
  float distSquared = distanceSquared(center, closest);
  return distSquared <= radius * radius;
}

bool CanvasHitTester::doesBoxIntersectTriangle(const glm::vec2& boxMin,
                                                const glm::vec2& boxMax,
                                                const glm::vec2& v0,
                                                const glm::vec2& v1,
                                                const glm::vec2& v2) {
  // Check if any triangle vertex is inside the box
  auto isPointInBox = [&](const glm::vec2& p) {
    return p.x >= boxMin.x && p.x <= boxMax.x && p.y >= boxMin.y &&
           p.y <= boxMax.y;
  };

  if (isPointInBox(v0) || isPointInBox(v1) || isPointInBox(v2)) {
    return true;
  }

  // Check if any box corner is inside the triangle
  auto boxCorners = {glm::vec2(boxMin.x, boxMin.y), glm::vec2(boxMax.x, boxMin.y),
                     glm::vec2(boxMax.x, boxMax.y),
                     glm::vec2(boxMin.x, boxMax.y)};

  for (const auto& corner : boxCorners) {
    if (isPointInTriangle(corner, v0, v1, v2, 0.0f)) {
      return true;
    }
  }

  // Check if any triangle edge intersects any box edge
  auto triangleEdges = {std::make_pair(v0, v1), std::make_pair(v1, v2),
                       std::make_pair(v2, v0)};

  auto boxEdges = {std::make_pair(glm::vec2(boxMin.x, boxMin.y),
                                  glm::vec2(boxMax.x, boxMin.y)),
                   std::make_pair(glm::vec2(boxMax.x, boxMin.y),
                                  glm::vec2(boxMax.x, boxMax.y)),
                   std::make_pair(glm::vec2(boxMax.x, boxMax.y),
                                  glm::vec2(boxMin.x, boxMax.y)),
                   std::make_pair(glm::vec2(boxMin.x, boxMax.y),
                                  glm::vec2(boxMin.x, boxMin.y))};

  for (const auto& triEdge : triangleEdges) {
    for (const auto& boxEdge : boxEdges) {
      if (segmentsIntersect(triEdge.first, triEdge.second, boxEdge.first,
                            boxEdge.second)) {
        return true;
      }
    }
  }

  return false;
}

bool CanvasHitTester::doesBoxIntersectPolygon(
    const glm::vec2& boxMin, const glm::vec2& boxMax,
    const std::vector<glm::vec2>& vertices) {
  if (vertices.size() < 3) {
    return false;
  }

  // Check if any vertex is inside the box
  auto isPointInBox = [&](const glm::vec2& p) {
    return p.x >= boxMin.x && p.x <= boxMax.x && p.y >= boxMin.y &&
           p.y <= boxMax.y;
  };

  for (const auto& vertex : vertices) {
    if (isPointInBox(vertex)) {
      return true;
    }
  }

  // Check if any box corner is inside the polygon
  auto boxCorners = {glm::vec2(boxMin.x, boxMin.y), glm::vec2(boxMax.x, boxMin.y),
                     glm::vec2(boxMax.x, boxMax.y),
                     glm::vec2(boxMin.x, boxMax.y)};

  for (const auto& corner : boxCorners) {
    if (isPointInPolygon(corner, vertices, 0.0f)) {
      return true;
    }
  }

  // Check if any polygon edge intersects any box edge
  auto boxEdges = {std::make_pair(glm::vec2(boxMin.x, boxMin.y),
                                  glm::vec2(boxMax.x, boxMin.y)),
                   std::make_pair(glm::vec2(boxMax.x, boxMin.y),
                                  glm::vec2(boxMax.x, boxMax.y)),
                   std::make_pair(glm::vec2(boxMax.x, boxMax.y),
                                  glm::vec2(boxMin.x, boxMax.y)),
                   std::make_pair(glm::vec2(boxMin.x, boxMax.y),
                                  glm::vec2(boxMin.x, boxMin.y))};

  size_t n = vertices.size();
  for (size_t i = 0; i < n; ++i) {
    const glm::vec2& v1 = vertices[i];
    const glm::vec2& v2 = vertices[(i + 1) % n];

    for (const auto& boxEdge : boxEdges) {
      if (segmentsIntersect(v1, v2, boxEdge.first, boxEdge.second)) {
        return true;
      }
    }
  }

  return false;
}

float CanvasHitTester::distanceSquared(const glm::vec2& a, const glm::vec2& b) {
  glm::vec2 diff = a - b;
  return glm::dot(diff, diff);
}

bool CanvasHitTester::segmentsIntersect(const glm::vec2& p1, const glm::vec2& p2,
                                        const glm::vec2& p3, const glm::vec2& p4) {
  // Check if line segments p1-p2 and p3-p4 intersect
  float d = (p2.x - p1.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p4.x - p3.x);

  if (std::abs(d) < 1e-6f) {
    return false; // Parallel or collinear
  }

  float u = ((p3.x - p1.x) * (p4.y - p3.y) - (p3.y - p1.y) * (p4.x - p3.x)) / d;
  float v = ((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) / d;

  return u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f;
}

} // namespace view
