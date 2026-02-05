#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace model {

// TODO: Add precision selection, try C++20 concepts
struct Point {
  float x{0.f};
  float y{0.f};
  float z{0.f};
};

struct Triangle {
  inline static const std::string name = "Triangle";
  Point first;
  Point second;
  Point third;
};

struct Quad {
  inline static const std::string name = "Quad";
  Point first;
  Point second;
  Point third;
  Point fourth;
};

struct Circle {
  inline static const std::string name = "Circle";
  Point center;
  float radius;
};

struct Ngon {
  inline static const std::string name = "Ngon";
  Point center;
  Point first;
  float n;
  float radius;
};

struct CurveBezier3 {
  inline static const std::string name = "CurveBezier3";
  Point start;
  Point end;
  Point first;
};

struct CurveBezier4 {
  inline static const std::string name = "CurveBezier4";
  Point start;
  Point end;
  Point first;
  Point second;
};

enum class isScribed { no, inscribed, circumscribed };

template <isScribed _scribed> struct ScribedTraits {
  static const bool value = true;
  float angle = 0.0f; // in radians, start from 0 at positive x axis across
                      // positive y to positive x = 2*pi, counterclock-wise
  float r = 0.0f;
};
template <> struct ScribedTraits<isScribed::no> {
  static const bool value = false;
};

// Intrusive structure that stores service data about a specific figure
template <class _T, isScribed _scribed = isScribed::no>
struct Figure : public _T {
private:
  inline static uint32_t counter_{0};

public:
  // const causes C2280 in MSVC because the copy assignment constructor
  // is automatically deleted
  uint32_t id_{};
  std::string name_{_T::name + " #" + std::to_string(id_)};
  [[no_unique_address]] ScribedTraits<_scribed> Round_;
  Figure(float x = 0.0f, float y = 0.0f, float z = 0.0f) {
    id_ = counter_++;
    if constexpr (Round_.value) {
      if constexpr (std::is_same_v<_T, Triangle>) {
        spdlog::info("triangle by round");
      }
    } else {
      if constexpr (std::is_same_v<_T, Triangle>) {
        spdlog::info("triangle by points");
      }
    }
  }
};

class Memento;

/**
 * @brief Complete structure that stores everything
 * @details Stores:
 * - all figures as variants
 * - coordinate system
 * - mouse coordinates
 * - invariant view-projection and model matrices
 * @note Refactoring may be required to switch to interfaces in the future
 */
class FlatFigures {
public:
  /// Add imaginary points for attachments in future implementation
  using allFigures_t =
      std::variant<Figure<Triangle>, Figure<Triangle, isScribed::inscribed>,
                   Figure<Triangle, isScribed::circumscribed>, Figure<Quad>,
                   Figure<Quad, isScribed::inscribed>,
                   Figure<Quad, isScribed::circumscribed>, Figure<Circle>,
                   Figure<Ngon, isScribed::inscribed>,
                   Figure<Ngon, isScribed::circumscribed>, Figure<CurveBezier3>,
                   Figure<CurveBezier4>>;

private:
  std::vector<allFigures_t> Figures_;
  glm::mat4
      inverseMVP_{}; // = glm::inverse(MVP); // = projection * view * model

public:
  glm::mat4 model_{1.0f};
  glm::mat4 projection_{1.0f};

  struct Camera_t {
    glm::vec3 position{0.0f, 0.0f, -1.0f};
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
  };

  Camera_t camera_{}; // view = glm::lookAt(camera_.position, camera_.target,
                      // camera_.up);

  struct Mouse_t {
    glm::vec2 NDC{0.0f, 0.0f};
    glm::vec2 clipSpace{0.0f, 0.0f};
    glm::vec2 viewSpace{0.0f, 0.0f};
    glm::vec2 worldSpace{0.0f, 0.0f};
  };

  Mouse_t mouse_{};

  /**
   * @brief Sets mouse coordinates from screen space and stores all coordinate
   * space transformations
   * @param absoluteMousePos Screen space coordinates in pixels [0, width] x [0,
   * height]
   * @note This method transforms screen coordinates through the full pipeline:
   *       Screen -> NDC -> Clip -> View -> World -> Model
   *       The transformed coordinates are stored in the mouse_ member for later
   * use
   */
  void setMouseCoordinates(glm::vec2 absoluteMousePos) {
    // Step 1: Transform screen coordinates to NDC (Normalized Device
    // Coordinates) Screen coordinates range from [0, width] and [0, height] NDC
    // ranges from [-1, 1] for both x and y Note: This requires knowing the
    // viewport size, which is not available here For now, we'll assume the
    // input is already in NDC space [-1, 1]
    // TODO: Pass viewport size to properly convert from screen pixels to NDC

    glm::vec4 ndcPosition =
        glm::vec4(absoluteMousePos.x, absoluteMousePos.y, 0.0f, 1.0f);

    // Step 2: Transform NDC to clip space (before perspective divide)
    // In OpenGL, NDC and clip space are related by the w component
    glm::vec4 clipPosition = ndcPosition;

    // Step 3: Transform through inverse matrices to get world space
    glm::mat4 inverseProjection = glm::inverse(projection_);
    glm::mat4 inverseView = glm::inverse(getView());
    glm::mat4 inverseModel = glm::inverse(model_);

    // Undo projection: Clip -> View
    glm::vec4 viewPosition = inverseProjection * clipPosition;

    // Store NDC coordinates (input coordinates assumed to be in NDC space)
    mouse_.NDC = glm::vec2(ndcPosition.x, ndcPosition.y);

    // Store clip space coordinates (normalized to [0, 1] from [-1, 1])
    mouse_.clipSpace =
        glm::vec2((ndcPosition.x + 1.0f) * 0.5f, (ndcPosition.y + 1.0f) * 0.5f);

    // Store view space coordinates
    mouse_.viewSpace = glm::vec2(viewPosition.x, viewPosition.y);

    // Undo view: View -> World
    glm::vec4 worldPosition = inverseView * viewPosition;
    mouse_.worldSpace = glm::vec2(worldPosition.x, worldPosition.y);
  }

  glm::mat4 getInverseMVP() {
    inverseMVP_ = glm::inverse(projection_ * getView() * model_);
    return inverseMVP_;
  }

  glm::mat4 getView() {
    return glm::lookAt(camera_.position, camera_.target, camera_.up);
  }

  allFigures_t getFlatFigure(uint32_t id) { return Figures_[id]; }

  /**
   * @brief Finds a figure by coordinates in world space
   * @param coords World space coordinates to search for figures near
   * @param delta Maximum distance in world units to consider a figure as
   * "found"
   * @return The found figure, or std::nullopt if no figure is within delta
   * distance
   * @note The coordinates parameter is expected to be in world space
   *       Use setMouseCoordinates() to transform screen coordinates to world
   * space first
   */
  std::optional<allFigures_t> findFlatFigureByCoords(glm::vec2 coords,
                                                     float delta) {
    if (Figures_.empty()) {
      return std::nullopt;
    }
    for (const auto &figure : Figures_) {
      // Check each figure type for proximity to the given coordinates
      if (std::holds_alternative<Figure<Triangle>>(figure)) {
        auto triangle = std::get<Figure<Triangle>>(figure);
        // Check if coordinates are within delta distance of any triangle vertex
        if (std::abs(triangle.first.x - coords.x) < delta &&
            std::abs(triangle.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.second.x - coords.x) < delta &&
            std::abs(triangle.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.third.x - coords.x) < delta &&
            std::abs(triangle.third.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Triangle, isScribed::inscribed>>(
                     figure)) {
        auto triangle =
            std::get<Figure<Triangle, isScribed::inscribed>>(figure);
        // Check if coordinates are within delta distance of any triangle vertex
        if (std::abs(triangle.first.x - coords.x) < delta &&
            std::abs(triangle.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.second.x - coords.x) < delta &&
            std::abs(triangle.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.third.x - coords.x) < delta &&
            std::abs(triangle.third.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<
                     Figure<Triangle, isScribed::circumscribed>>(figure)) {
        auto triangle =
            std::get<Figure<Triangle, isScribed::circumscribed>>(figure);
        // Check if coordinates are within delta distance of any triangle vertex
        if (std::abs(triangle.first.x - coords.x) < delta &&
            std::abs(triangle.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.second.x - coords.x) < delta &&
            std::abs(triangle.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(triangle.third.x - coords.x) < delta &&
            std::abs(triangle.third.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Quad>>(figure)) {
        auto quad = std::get<Figure<Quad>>(figure);
        // Check if coordinates are within delta distance of any quad vertex
        if (std::abs(quad.first.x - coords.x) < delta &&
            std::abs(quad.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.second.x - coords.x) < delta &&
            std::abs(quad.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.third.x - coords.x) < delta &&
            std::abs(quad.third.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.fourth.x - coords.x) < delta &&
            std::abs(quad.fourth.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Quad, isScribed::inscribed>>(
                     figure)) {
        auto quad = std::get<Figure<Quad, isScribed::inscribed>>(figure);
        // Check if coordinates are within delta distance of any quad vertex
        if (std::abs(quad.first.x - coords.x) < delta &&
            std::abs(quad.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.second.x - coords.x) < delta &&
            std::abs(quad.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.third.x - coords.x) < delta &&
            std::abs(quad.third.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.fourth.x - coords.x) < delta &&
            std::abs(quad.fourth.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Quad, isScribed::circumscribed>>(
                     figure)) {
        auto quad = std::get<Figure<Quad, isScribed::circumscribed>>(figure);
        // Check if coordinates are within delta distance of any quad vertex
        if (std::abs(quad.first.x - coords.x) < delta &&
            std::abs(quad.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.second.x - coords.x) < delta &&
            std::abs(quad.second.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.third.x - coords.x) < delta &&
            std::abs(quad.third.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(quad.fourth.x - coords.x) < delta &&
            std::abs(quad.fourth.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Circle>>(figure)) {
        auto circle = std::get<Figure<Circle>>(figure);
        // Check if coordinates are within delta distance of circle center
        float distance = std::sqrt(std::pow(circle.center.x - coords.x, 2) +
                                   std::pow(circle.center.y - coords.y, 2));
        if (distance < circle.radius + delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Ngon, isScribed::inscribed>>(
                     figure)) {
        auto ngon = std::get<Figure<Ngon, isScribed::inscribed>>(figure);
        // Check if coordinates are within delta distance of ngon center
        float distance = std::sqrt(std::pow(ngon.center.x - coords.x, 2) +
                                   std::pow(ngon.center.y - coords.y, 2));
        if (distance < ngon.radius + delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<Ngon, isScribed::circumscribed>>(
                     figure)) {
        auto ngon = std::get<Figure<Ngon, isScribed::circumscribed>>(figure);
        // Check if coordinates are within delta distance of ngon center
        float distance = std::sqrt(std::pow(ngon.center.x - coords.x, 2) +
                                   std::pow(ngon.center.y - coords.y, 2));
        if (distance < ngon.radius + delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<CurveBezier3>>(figure)) {
        auto curve = std::get<Figure<CurveBezier3>>(figure);
        // Check if coordinates are within delta distance of any curve point
        if (std::abs(curve.start.x - coords.x) < delta &&
            std::abs(curve.start.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(curve.end.x - coords.x) < delta &&
            std::abs(curve.end.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(curve.first.x - coords.x) < delta &&
            std::abs(curve.first.y - coords.y) < delta) {
          return figure;
        }
      } else if (std::holds_alternative<Figure<CurveBezier4>>(figure)) {
        auto curve = std::get<Figure<CurveBezier4>>(figure);
        // Check if coordinates are within delta distance of any curve point
        if (std::abs(curve.start.x - coords.x) < delta &&
            std::abs(curve.start.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(curve.end.x - coords.x) < delta &&
            std::abs(curve.end.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(curve.first.x - coords.x) < delta &&
            std::abs(curve.first.y - coords.y) < delta) {
          return figure;
        }
        if (std::abs(curve.second.x - coords.x) < delta &&
            std::abs(curve.second.y - coords.y) < delta) {
          return figure;
        }
      }
    }
    return std::nullopt;
  }

private:
  template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
  };
  template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

public:
  FlatFigures() = default;
  ~FlatFigures() = default;

  template <class T> void createFigure(float x, float y) {
    Figures_.push_back(Figure<T>{});
    auto figure = Figures_.back();
    if constexpr (std::is_same_v<T, Triangle>) {
      spdlog::info("Triangle in model");
      // Save all three points in one place here
    }
  }

  /*std::unique_ptr<Memento> createMemento() {
          return std::make_unique<Memento>(Memento(Figures_));
  }

  void restoreFromMemento(const Memento& Mem) {
          Figures_ = Mem.Figures_;
  }*/
};

class Memento {
  friend class FlatFigures;

private:
  std::vector<FlatFigures::allFigures_t> Figures_;

public:
  Memento(const std::vector<FlatFigures::allFigures_t> &Figures)
      : Figures_(Figures) {}
  ~Memento() = default;
};

} // namespace model

/// Consider using polymorphism with a base Figure class instead of variants
/// and a vector of polymorphic pointers. This would reduce template usage
/// and simplify the code structure.