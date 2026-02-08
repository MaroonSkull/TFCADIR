#pragma once

#include <cmath>
#include <memory>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>

// Forward declaration for model interface
namespace model {
class FlatFigures;
template <class T> struct Figure;
struct Triangle;
struct Quad;
struct Circle;
struct Ngon;
} // namespace model

namespace controller {

/**
 * @brief Abstract base class for figure creators using Factory Pattern
 * @details Provides interface for creating different types of geometric figures
 *          from collected 2D points in world space coordinates
 */
class FigureCreator {
public:
  /// Pointer to the model for adding figures
  model::FlatFigures *model_;

  /**
   * @brief Constructs a figure creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit FigureCreator(model::FlatFigures *model) : model_(model) {}

  virtual ~FigureCreator() = default;

  /**
   * @brief Returns the number of points required for this figure type
   * @return Number of points needed to create the figure
   */
  virtual size_t getRequiredPoints() const = 0;

  /**
   * @brief Validates if a new point can be added to the current points
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if the point is valid, false otherwise
   * @note Default implementation accepts all points
   */
  virtual bool validatePoint(const std::vector<glm::vec2> &points,
                             const glm::vec2 &newPoint) const {
    (void)points;
    (void)newPoint;
    return true;
  }

  /**
   * @brief Checks if the figure can be completed with current points
   * @param points Current set of collected points
   * @return true if enough points are collected, false otherwise
   */
  virtual bool canComplete(const std::vector<glm::vec2> &points) const {
    return points.size() >= getRequiredPoints();
  }

  /**
   * @brief Updates the preview rendering with current points
   * @param points Current set of collected points
   * @note This is a placeholder for future preview rendering implementation
   */
  virtual void updatePreview(const std::vector<glm::vec2> &points) const {
    (void)points;
    // Preview rendering to be implemented with OpenGL integration
  }

  /**
   * @brief Creates the final figure from collected points and adds it to the
   * model
   * @param points Vector of 2D points in world space coordinates
   * @throws std::runtime_error if points vector is invalid or insufficient
   */
  virtual void createFigure(const std::vector<glm::vec2> &points) = 0;

  /**
   * @brief Returns the display name for this figure type
   * @return Human-readable name suitable for UI display
   */
  virtual std::string displayName() const = 0;
};

/**
 * @brief Creator for line figures (2 points)
 * @details Creates a line segment between two endpoints
 */
class LineCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a line creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit LineCreator(model::FlatFigures *model) : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for line creation
   * @return Always returns 2 (start and end points)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for line figures
   * @return String "Line"
   */
  std::string displayName() const override { return "Line"; }

  /**
   * @brief Validates that the two points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if point is distinct from existing points, false otherwise
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    for (const auto &p : points) {
      if (glm::length(p - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for line creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for line
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a line figure from two points
   * @param points Vector containing exactly 2 points
   * @throws std::runtime_error if points size is not 2
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for triangle figures defined by center and radius (2 points)
 * @details Creates an equilateral triangle with center at first point
 *          and second point defining the distance to vertices
 */
class TriangleByCenterCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a triangle by center creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit TriangleByCenterCreator(model::FlatFigures *model)
      : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for triangle creation
   * @return Always returns 2 (center and radius point)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for triangle figures
   * @return String "Triangle (Center)"
   */
  std::string displayName() const override { return "Triangle (Center)"; }

  /**
   * @brief Validates that center and radius points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if radius > 0, false if center equals radius point
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    if (!points.empty()) {
      if (glm::length(points[0] - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for triangle creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for triangle
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a triangle from center point and radius point
   * @param points Vector containing center [0] and radius point [1]
   * @throws std::runtime_error if points size is not 2
   * @note Calculates three vertices equidistant from center
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for triangle figures defined by three corner points (3 points)
 * @details Creates a triangle with vertices at the three collected points
 */
class TriangleByCornersCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a triangle by corners creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit TriangleByCornersCreator(model::FlatFigures *model)
      : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for triangle creation
   * @return Always returns 3 (three corner points)
   */
  size_t getRequiredPoints() const override { return 3; }

  /**
   * @brief Returns the display name for triangle figures
   * @return String "Triangle (Corners)"
   */
  std::string displayName() const override { return "Triangle (Corners)"; }

  /**
   * @brief Validates that three points form a valid triangle (not collinear)
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if points are not collinear, false otherwise
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override;

  /**
   * @brief Checks if enough points are collected for triangle creation
   * @param points Current set of collected points
   * @return true if 3 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 3;
  }

  /**
   * @brief Updates the preview rendering for triangle
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a triangle from three corner points
   * @param points Vector containing exactly 3 corner points
   * @throws std::runtime_error if points size is not 3 or points are collinear
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for square figures defined by center and size (2 points)
 * @details Creates a square with center at first point
 *          and second point defining half the diagonal length
 */
class SquareByCenterCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a square by center creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit SquareByCenterCreator(model::FlatFigures *model)
      : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for square creation
   * @return Always returns 2 (center and corner point)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for square figures
   * @return String "Square (Center)"
   */
  std::string displayName() const override { return "Square (Center)"; }

  /**
   * @brief Validates that center and corner points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if size > 0, false if center equals corner
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    if (!points.empty()) {
      if (glm::length(points[0] - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for square creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for square
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a square from center point and corner point
   * @param points Vector containing center [0] and corner [1]
   * @throws std::runtime_error if points size is not 2
   * @note Calculates four corners from center and half-diagonal
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for square figures defined by two diagonal corners (2 points)
 * @details Creates a square with the two points as opposite corners
 */
class SquareByCornersCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a square by corners creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit SquareByCornersCreator(model::FlatFigures *model)
      : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for square creation
   * @return Always returns 2 (diagonal corner points)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for square figures
   * @return String "Square (Corners)"
   */
  std::string displayName() const override { return "Square (Corners)"; }

  /**
   * @brief Validates that diagonal points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if diagonal has length > 0, false otherwise
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    if (!points.empty()) {
      if (glm::length(points[0] - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for square creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for square
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a square from two diagonal corner points
   * @param points Vector containing exactly 2 diagonal corner points
   * @throws std::runtime_error if points size is not 2
   * @note Calculates all four corners from the diagonal endpoints
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for regular n-gon figures defined by center and radius (2
 * points)
 * @details Creates a regular polygon with specified number of sides
 */
class NgonByCenterCreator : public FigureCreator {
private:
  /// Number of sides for the n-gon
  int numSides_;

public:
  /**
   * @brief Constructs an n-gon creator with specified side count
   * @param model Pointer to the FlatFigures model (non-owning)
   * @param numSides Number of sides for the polygon (default: 6 for hexagon)
   * @throws std::invalid_argument if numSides < 3
   */
  NgonByCenterCreator(model::FlatFigures *model, int numSides = 6)
      : FigureCreator(model), numSides_(numSides) {
    if (numSides < 3) {
      throw std::invalid_argument(
          "NgonByCenterCreator: numSides must be at least 3, got " +
          std::to_string(numSides));
    }
  }

  /**
   * @brief Returns the number of points required for n-gon creation
   * @return Always returns 2 (center and radius point)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for n-gon figures
   * @return String describing the n-gon (e.g., "Hexagon", "Pentagon")
   */
  std::string displayName() const override {
    switch (numSides_) {
    case 3:
      return "Triangle";
    case 4:
      return "Square";
    case 5:
      return "Pentagon";
    case 6:
      return "Hexagon";
    case 7:
      return "Heptagon";
    case 8:
      return "Octagon";
    case 9:
      return "Nonagon";
    case 10:
      return "Decagon";
    default:
      return "N-gon (" + std::to_string(numSides_) + " sides)";
    }
  }

  /**
   * @brief Validates that center and radius points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if radius > 0, false if center equals radius point
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    if (!points.empty()) {
      if (glm::length(points[0] - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for n-gon creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for n-gon
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates an n-gon from center point and radius point
   * @param points Vector containing center [0] and radius point [1]
   * @throws std::runtime_error if points size is not 2
   * @note Calculates vertices evenly distributed around the center
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

/**
 * @brief Creator for circle figures defined by center and radius (2 points)
 * @details Creates a circle with center at first point
 *          and second point defining the radius
 */
class CircleByCenterCreator : public FigureCreator {
public:
  /**
   * @brief Constructs a circle by center creator with a model reference
   * @param model Pointer to the FlatFigures model (non-owning)
   */
  explicit CircleByCenterCreator(model::FlatFigures *model)
      : FigureCreator(model) {}

  /**
   * @brief Returns the number of points required for circle creation
   * @return Always returns 2 (center and radius point)
   */
  size_t getRequiredPoints() const override { return 2; }

  /**
   * @brief Returns the display name for circle figures
   * @return String "Circle"
   */
  std::string displayName() const override { return "Circle"; }

  /**
   * @brief Validates that center and radius points are not identical
   * @param points Current set of collected points
   * @param newPoint The new point to validate
   * @return true if radius > 0, false if center equals radius point
   */
  bool validatePoint(const std::vector<glm::vec2> &points,
                     const glm::vec2 &newPoint) const override {
    if (!points.empty()) {
      if (glm::length(points[0] - newPoint) < 1e-6f) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Checks if enough points are collected for circle creation
   * @param points Current set of collected points
   * @return true if 2 points are collected, false otherwise
   */
  bool canComplete(const std::vector<glm::vec2> &points) const override {
    return points.size() >= 2;
  }

  /**
   * @brief Updates the preview rendering for circle
   * @param points Current set of collected points
   */
  void updatePreview(const std::vector<glm::vec2> &points) const override {
    (void)points;
    // Preview rendering to be implemented
  }

  /**
   * @brief Creates a circle from center point and radius point
   * @param points Vector containing center [0] and radius point [1]
   * @throws std::runtime_error if points size is not 2
   */
  void createFigure(const std::vector<glm::vec2> &points) override;
};

} // namespace controller
