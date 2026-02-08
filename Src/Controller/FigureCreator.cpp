#include "FigureCreator.hpp"

#include <stdexcept>

#include "Model/FlatFigure.hpp"

namespace controller {

/// Helper function to convert glm::vec2 to model::Point
static model::Point vec2ToPoint(const glm::vec2 &v) {
  return model::Point{v.x, v.y, 0.0f};
}

/// Helper function to check if three points are collinear
static bool areCollinear(const glm::vec2 &p1, const glm::vec2 &p2,
                         const glm::vec2 &p3, float epsilon = 1e-6f) {
  // Calculate the area of the triangle formed by the three points
  // If area is close to zero, points are collinear
  float area =
      std::abs((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y));
  return area < epsilon;
}

/// Helper function to check if four points form a valid square
static bool isValidSquare(const glm::vec2 &p1, const glm::vec2 &p2,
                          const glm::vec2 &p3, const glm::vec2 &p4,
                          float epsilon = 1e-6f) {
  // Calculate all distances
  float d12 = glm::length(p2 - p1);
  float d23 = glm::length(p3 - p2);
  float d34 = glm::length(p4 - p3);
  float d41 = glm::length(p1 - p4);
  float d13 = glm::length(p3 - p1); // diagonal
  float d24 = glm::length(p4 - p2); // diagonal

  // All sides should be approximately equal
  float side_avg = (d12 + d23 + d34 + d41) / 4.0f;
  float side_dev = std::abs(d12 - side_avg) + std::abs(d23 - side_avg) +
                   std::abs(d34 - side_avg) + std::abs(d41 - side_avg);

  // Diagonals should be approximately equal
  float diagonal_avg = (d13 + d24) / 2.0f;
  float diagonal_dev =
      std::abs(d13 - diagonal_avg) + std::abs(d24 - diagonal_avg);

  // Side and diagonal should have the correct ratio (sqrt(2))
  float ratio = diagonal_avg / side_avg;
  float expected_ratio = std::sqrt(2.0f);
  float ratio_dev = std::abs(ratio - expected_ratio);

  return side_dev < epsilon * 4 && diagonal_dev < epsilon * 2 &&
         ratio_dev < epsilon * 10;
}

void LineCreator::createFigure(const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error("Line requires exactly 2 points, got " +
                             std::to_string(points.size()));
  }

  // For now, we'll create a Triangle with two identical points
  // TODO: Add proper Line support to the model
  auto figure = std::make_shared<model::Figure<model::Triangle>>(
      points[0].x, points[0].y, 0.0f, model::isScribed::no);
  figure->first = vec2ToPoint(points[0]);
  figure->second = vec2ToPoint(points[1]);
  figure->third = vec2ToPoint(points[1]); // Same as second point for now

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

void TriangleByCenterCreator::createFigure(
    const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error(
        "Triangle by center requires exactly 2 points, got " +
        std::to_string(points.size()));
  }

  const glm::vec2 &center = points[0];
  const glm::vec2 &radiusPoint = points[1];
  float radius = glm::length(radiusPoint - center);

  // Calculate three vertices equidistant from center
  float angleOffset =
      std::atan2(radiusPoint.y - center.y, radiusPoint.x - center.x);

  auto figure = std::make_shared<model::Figure<model::Triangle>>(
      center.x, center.y, 0.0f, model::isScribed::no);

  for (int i = 0; i < 3; ++i) {
    float angle = angleOffset + (2.0f * std::numbers::pi_v<float> / 3.0f) * i;
    glm::vec2 vertex =
        center + radius * glm::vec2(std::cos(angle), std::sin(angle));

    if (i == 0) {
      figure->first = vec2ToPoint(vertex);
    } else if (i == 1) {
      figure->second = vec2ToPoint(vertex);
    } else {
      figure->third = vec2ToPoint(vertex);
    }
  }

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

bool TriangleByCornersCreator::validatePoint(
    const std::vector<glm::vec2> &points, const glm::vec2 &newPoint) const {
  if (points.size() < 2) {
    return true;
  }

  // If we have 2 points already, check if the third point would make them
  // collinear
  if (points.size() == 2) {
    if (areCollinear(points[0], points[1], newPoint)) {
      return false;
    }
  }

  return true;
}

void TriangleByCornersCreator::createFigure(
    const std::vector<glm::vec2> &points) {
  if (points.size() != 3) {
    throw std::runtime_error(
        "Triangle by corners requires exactly 3 points, got " +
        std::to_string(points.size()));
  }

  // Check if points are collinear
  if (areCollinear(points[0], points[1], points[2])) {
    throw std::runtime_error("Triangle points are collinear");
  }

  auto figure = std::make_shared<model::Figure<model::Triangle>>(
      points[0].x, points[0].y, 0.0f, model::isScribed::no);
  figure->first = vec2ToPoint(points[0]);
  figure->second = vec2ToPoint(points[1]);
  figure->third = vec2ToPoint(points[2]);

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

void SquareByCenterCreator::createFigure(const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error(
        "Square by center requires exactly 2 points, got " +
        std::to_string(points.size()));
  }

  const glm::vec2 &center = points[0];
  const glm::vec2 &corner = points[1];
  float halfDiagonal = glm::length(corner - center);

  // Calculate the four corners from center and half-diagonal
  // The angle to the first corner determines the rotation
  float angleOffset = std::atan2(corner.y - center.y, corner.x - center.x);

  auto figure = std::make_shared<model::Figure<model::Quad>>(
      center.x, center.y, 0.0f, model::isScribed::no);

  for (int i = 0; i < 4; ++i) {
    float angle = angleOffset + (2.0f * std::numbers::pi_v<float> / 4.0f) * i;
    glm::vec2 vertex =
        center + halfDiagonal * glm::vec2(std::cos(angle), std::sin(angle));

    if (i == 0) {
      figure->first = vec2ToPoint(vertex);
    } else if (i == 1) {
      figure->second = vec2ToPoint(vertex);
    } else if (i == 2) {
      figure->third = vec2ToPoint(vertex);
    } else {
      figure->fourth = vec2ToPoint(vertex);
    }
  }

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

void SquareByCornersCreator::createFigure(
    const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error(
        "Square by corners requires exactly 2 diagonal points, got " +
        std::to_string(points.size()));
  }

  const glm::vec2 &p1 = points[0];
  const glm::vec2 &p2 = points[1];

  // Calculate the center and half-diagonal
  glm::vec2 center = (p1 + p2) / 2.0f;
  float halfDiagonal = glm::length(p2 - p1) / 2.0f;

  // Calculate the angle to the first diagonal corner
  float angleOffset = std::atan2(p1.y - center.y, p1.x - center.x);

  auto figure = std::make_shared<model::Figure<model::Quad>>(
      center.x, center.y, 0.0f, model::isScribed::no);

  for (int i = 0; i < 4; ++i) {
    float angle = angleOffset + (2.0f * std::numbers::pi_v<float> / 4.0f) * i;
    glm::vec2 vertex =
        center + halfDiagonal * glm::vec2(std::cos(angle), std::sin(angle));

    if (i == 0) {
      figure->first = vec2ToPoint(vertex);
    } else if (i == 1) {
      figure->second = vec2ToPoint(vertex);
    } else if (i == 2) {
      figure->third = vec2ToPoint(vertex);
    } else {
      figure->fourth = vec2ToPoint(vertex);
    }
  }

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

void NgonByCenterCreator::createFigure(const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error("N-gon requires exactly 2 points, got " +
                             std::to_string(points.size()));
  }

  const glm::vec2 &center = points[0];
  const glm::vec2 &radiusPoint = points[1];
  float radius = glm::length(radiusPoint - center);

  // Calculate the angle offset from center to radius point
  float angleOffset =
      std::atan2(radiusPoint.y - center.y, radiusPoint.x - center.x);

  auto figure = std::make_shared<model::Figure<model::Ngon>>(
      center.x, center.y, 0.0f, model::isScribed::no);
  figure->center = vec2ToPoint(center);
  figure->radius = radius;
  figure->n = static_cast<float>(numSides_);

  // Calculate first vertex
  glm::vec2 firstVertex =
      center + radius * glm::vec2(std::cos(angleOffset), std::sin(angleOffset));
  figure->first = vec2ToPoint(firstVertex);

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

void CircleByCenterCreator::createFigure(const std::vector<glm::vec2> &points) {
  if (points.size() != 2) {
    throw std::runtime_error("Circle requires exactly 2 points, got " +
                             std::to_string(points.size()));
  }

  const glm::vec2 &center = points[0];
  const glm::vec2 &radiusPoint = points[1];
  float radius = glm::length(radiusPoint - center);

  auto figure = std::make_shared<model::Figure<model::Circle>>(
      center.x, center.y, 0.0f, model::isScribed::no);
  figure->center = vec2ToPoint(center);
  figure->radius = radius;

  if (model_) {
    model_->addFigurePtr(figure);
  }
}

} // namespace controller
