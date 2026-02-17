#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "IModel.hpp"

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

/**
 * @brief Intrusive structure that stores service data about a specific figure
 * @details This template class wraps geometric figure types and adds
 *          identification, naming, and positioning capabilities through
 *          the IFigure interface. It supports both regular and scribed
 *          (inscribed/circumscribed) figure variants using runtime enum.
 * @tparam _T The underlying geometric figure type (Triangle, Quad, Circle,
 * etc.)
 */
template <class _T> struct Figure : public _T, public IFigure {
private:
  inline static uint32_t counter_{0};
  glm::vec3 position_{0.0f, 0.0f, 0.0f};

  /// Appearance properties (stroke/fill colors, line width)
  Appearance appearance_{};

  /// Layer properties (layer index, visibility, lock state)
  LayerProperties layerProperties_{};

  /// Creation timestamp (ISO 8601 format)
  std::string creationDate_;

public:
  /// Unique identifier for this figure instance
  uint32_t id_{};

  /// Human-readable name for this figure
  std::string name_{_T::name + " #" + std::to_string(id_)};

  /// Scribed state of the figure (no, inscribed, or circumscribed)
  isScribed scribed_{isScribed::no};

  /**
   * @brief Constructs a new Figure with optional position and scribed state
   * @param x X coordinate in world space
   * @param y Y coordinate in world space
   * @param z Z coordinate in world space (default 0 for 2D figures)
   * @param scribed Whether the figure is scribed (inscribed/circumscribed) or
   * not
   */
  Figure(float x = 0.0f, float y = 0.0f, float z = 0.0f,
         isScribed scribed = isScribed::no)
      : position_(x, y, z), scribed_(scribed) {
    id_ = counter_++;
    name_ = _T::name + " #" + std::to_string(id_);

    // Set creation timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
    creationDate_ = ss.str();

    if (scribed_ != isScribed::no) {
      if constexpr (std::is_same_v<_T, Triangle>) {
        spdlog::info("triangle by round");
      }
    } else {
      if constexpr (std::is_same_v<_T, Triangle>) {
        spdlog::info("triangle by points");
      }
    }
  }

  // IFigure interface implementation

  /**
   * @brief Gets the unique identifier for this figure
   * @return The figure's unique ID
   */
  uint32_t getId() const override { return id_; }

  /**
   * @brief Gets the name of this figure
   * @return The figure's name
   */
  const std::string &getName() const override { return name_; }

  /**
   * @brief Sets the name of this figure
   * @param name The new name for the figure
   */
  void setName(const std::string &name) override { name_ = name; }

  /**
   * @brief Gets the position of this figure in 3D space
   * @return The figure's position as a 3D vector
   * @note For 2D figures, the z-component will typically be 0
   */
  glm::vec3 getPosition() const override { return position_; }

  /**
   * @brief Sets the position of this figure in 3D space
   * @param position The new position for the figure
   */
  void setPosition(const glm::vec3 &position) override { position_ = position; }

  /**
   * @brief Gets the bounding box of this figure
   * @return A pair of vec3 representing min and max bounds
   * @note The first element is the minimum corner, second is the maximum corner
   * @note This is a simplified implementation that returns a point-based bounds
   */
  std::pair<glm::vec3, glm::vec3> getBounds() const override {
    if constexpr (std::is_same_v<_T, Triangle>) {
      glm::vec3 min{std::min({this->first.x, this->second.x, this->third.x}),
                    std::min({this->first.y, this->second.y, this->third.y}),
                    std::min({this->first.z, this->second.z, this->third.z})};
      glm::vec3 max{std::max({this->first.x, this->second.x, this->third.x}),
                    std::max({this->first.y, this->second.y, this->third.y}),
                    std::max({this->first.z, this->second.z, this->third.z})};
      return {min, max};
    } else if constexpr (std::is_same_v<_T, Quad>) {
      glm::vec3 min{std::min({this->first.x, this->second.x, this->third.x,
                              this->fourth.x}),
                    std::min({this->first.y, this->second.y, this->third.y,
                              this->fourth.y}),
                    std::min({this->first.z, this->second.z, this->third.z,
                              this->fourth.z})};
      glm::vec3 max{std::max({this->first.x, this->second.x, this->third.x,
                              this->fourth.x}),
                    std::max({this->first.y, this->second.y, this->third.y,
                              this->fourth.y}),
                    std::max({this->first.z, this->second.z, this->third.z,
                              this->fourth.z})};
      return {min, max};
    } else if constexpr (std::is_same_v<_T, Circle>) {
      glm::vec3 min{this->center.x - this->radius,
                    this->center.y - this->radius, this->center.z};
      glm::vec3 max{this->center.x + this->radius,
                    this->center.y + this->radius, this->center.z};
      return {min, max};
    } else if constexpr (std::is_same_v<_T, Ngon>) {
      glm::vec3 min{this->center.x - this->radius,
                    this->center.y - this->radius, this->center.z};
      glm::vec3 max{this->center.x + this->radius,
                    this->center.y + this->radius, this->center.z};
      return {min, max};
    } else if constexpr (std::is_same_v<_T, CurveBezier3>) {
      glm::vec3 min{std::min({this->start.x, this->end.x, this->first.x}),
                    std::min({this->start.y, this->end.y, this->first.y}),
                    std::min({this->start.z, this->end.z, this->first.z})};
      glm::vec3 max{std::max({this->start.x, this->end.x, this->first.x}),
                    std::max({this->start.y, this->end.y, this->first.y}),
                    std::max({this->start.z, this->end.z, this->first.z})};
      return {min, max};
    } else if constexpr (std::is_same_v<_T, CurveBezier4>) {
      glm::vec3 min{
          std::min({this->start.x, this->end.x, this->first.x, this->second.x}),
          std::min({this->start.y, this->end.y, this->first.y, this->second.y}),
          std::min(
              {this->start.z, this->end.z, this->first.z, this->second.z})};
      glm::vec3 max{
          std::max({this->start.x, this->end.x, this->first.x, this->second.x}),
          std::max({this->start.y, this->end.y, this->first.y, this->second.y}),
          std::max(
              {this->start.z, this->end.z, this->first.z, this->second.z})};
      return {min, max};
    }
    // Default fallback
    return {position_, position_};
  }

  // === Appearance Properties ===

  glm::vec3 getStrokeColor() const override { return appearance_.strokeColor; }

  void setStrokeColor(const glm::vec3 &color) override {
    appearance_.strokeColor = color;
  }

  glm::vec3 getFillColor() const override { return appearance_.fillColor; }

  void setFillColor(const glm::vec3 &color) override {
    appearance_.fillColor = color;
  }

  Appearance getAppearance() const override { return appearance_; }

  void setAppearance(const Appearance &appearance) override {
    appearance_ = appearance;
  }

  // === Layer Properties ===

  int getLayer() const override { return layerProperties_.layerIndex; }

  void setLayer(int layerIndex) override {
    layerProperties_.layerIndex = layerIndex;
  }

  bool isVisible() const override { return layerProperties_.visible; }

  void setVisible(bool visible) override { layerProperties_.visible = visible; }

  bool isLocked() const override { return layerProperties_.locked; }

  void setLocked(bool locked) override { layerProperties_.locked = locked; }

  LayerProperties getLayerProperties() const override {
    return layerProperties_;
  }

  void setLayerProperties(const LayerProperties &props) override {
    layerProperties_ = props;
  }

  // === Creation Date ===

  std::string getCreationDate() const override { return creationDate_; }
};

/**
 * @brief Memento class for state persistence
 * @details Implements the Memento pattern to capture and restore
 *          the internal state of FlatFigures
 */
class Memento {
  friend class FlatFigures;

private:
  std::vector<std::shared_ptr<IFigure>> Figures_;

public:
  /**
   * @brief Constructs a memento with the current figures state
   * @param Figures The vector of figures to capture
   */
  Memento(const std::vector<std::shared_ptr<IFigure>> &Figures)
      : Figures_(Figures) {}

  ~Memento() = default;
};

/**
 * @brief Complete structure that stores everything
 * @details Stores:
 * - all figures as polymorphic shared_ptr<IFigure>
 * - coordinate system
 * - mouse coordinates
 * - invariant view-projection and model matrices
 * @note Now uses polymorphism instead of std::variant for better extensibility
 *       and implements the IModel interface for MVC architecture compliance
 */
class FlatFigures : public IModel {
private:
  std::vector<std::shared_ptr<IFigure>> Figures_;
  glm::mat4
      inverseMVP_{}; // = glm::inverse(MVP); // = projection * view * model

  /**
   * @brief Mouse coordinates across different coordinate spaces
   * @details Stores mouse coordinates transformed through the rendering
   * pipeline
   */
  struct Mouse_t {
    glm::vec2 NDC{0.0f, 0.0f};
    glm::vec2 clipSpace{0.0f, 0.0f};
    glm::vec2 viewSpace{0.0f, 0.0f};
    glm::vec2 worldSpace{0.0f, 0.0f};
  };

  Mouse_t mouse_{};

public:
  glm::mat4 model_{1.0f};
  glm::mat4 projection_{1.0f};

  /**
   * @brief Camera structure for view transformations
   * @details Stores position, target, and up vector for camera calculations
   */
  struct Camera_t {
    glm::vec3 position{0.0f, 0.0f, -1.0f};
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
  };

  Camera_t camera_{};

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

  /**
   * @brief Gets a figure by its index in the figures vector
   * @param id The index of the figure to retrieve
   * @return Shared pointer to the figure, or nullptr if index is invalid
   */
  std::shared_ptr<IFigure> getFlatFigure(uint32_t id) {
    spdlog::debug("[DEBUG] getFlatFigure called - id={}, Figures_.size()={}",
                  id, Figures_.size());
    if (id >= Figures_.size()) {
      spdlog::warn(
          "[DEBUG] getFlatFigure - id {} >= size {}, returning nullptr", id,
          Figures_.size());
      return nullptr;
    }
    auto figure = Figures_[id];
    if (figure) {
      spdlog::debug(
          "[DEBUG] getFlatFigure - returning figure with actual id={}",
          figure->getId());
    } else {
      spdlog::warn("[DEBUG] getFlatFigure - Figures_[{}] is nullptr!", id);
    }
    return figure;
  }

  /**
   * @brief Finds a figure by coordinates in world space
   * @param coords World space coordinates to search for figures near
   * @param delta Maximum distance in world units to consider a figure as
   * "found"
   * @return Shared pointer to the found figure, or nullptr if no figure is
   * within delta distance
   * @note The coordinates parameter is expected to be in world space
   *       Use setMouseCoordinates() to transform screen coordinates to world
   * space first
   * @note This method uses dynamic_cast to check specific figure types and
   *       performs hit-testing based on the figure's geometry
   */
  std::shared_ptr<IFigure> findFlatFigureByCoords(glm::vec2 coords,
                                                  float delta) {
    if (Figures_.empty()) {
      return nullptr;
    }
    for (const auto &figure : Figures_) {
      // Check Triangle
      if (auto tri = std::dynamic_pointer_cast<Figure<Triangle>>(figure)) {
        if (checkPointProximity(coords, delta, tri->first, tri->second,
                                tri->third)) {
          return figure;
        }
      }
      // Check Quad
      else if (auto quad = std::dynamic_pointer_cast<Figure<Quad>>(figure)) {
        if (checkPointProximity(coords, delta, quad->first, quad->second,
                                quad->third, quad->fourth)) {
          return figure;
        }
      }
      // Check Circle
      else if (auto circle =
                   std::dynamic_pointer_cast<Figure<Circle>>(figure)) {
        float distance = std::sqrt(std::pow(circle->center.x - coords.x, 2) +
                                   std::pow(circle->center.y - coords.y, 2));
        if (distance < circle->radius + delta) {
          return figure;
        }
      }
      // Check Ngon
      else if (auto ngon = std::dynamic_pointer_cast<Figure<Ngon>>(figure)) {
        float distance = std::sqrt(std::pow(ngon->center.x - coords.x, 2) +
                                   std::pow(ngon->center.y - coords.y, 2));
        if (distance < ngon->radius + delta) {
          return figure;
        }
      }
      // Check CurveBezier3
      else if (auto curve =
                   std::dynamic_pointer_cast<Figure<CurveBezier3>>(figure)) {
        if (checkPointProximity(coords, delta, curve->start, curve->end,
                                curve->first)) {
          return figure;
        }
      }
      // Check CurveBezier4
      else if (auto curve =
                   std::dynamic_pointer_cast<Figure<CurveBezier4>>(figure)) {
        if (checkPointProximity(coords, delta, curve->start, curve->end,
                                curve->first, curve->second)) {
          return figure;
        }
      }
    }
    return nullptr;
  }

private:
  /**
   * @brief Helper method to check if coordinates are near any of the given
   * points
   * @param coords The coordinates to check
   * @param delta Maximum distance to consider as "near"
   * @param points Variable number of points to check against
   * @return true if coordinates are within delta distance of any point
   */
  template <typename... Points>
  bool checkPointProximity(glm::vec2 coords, float delta,
                           const Points &...points) const {
    return ((std::abs(points.x - coords.x) < delta &&
             std::abs(points.y - coords.y) < delta) ||
            ...);
  }

public:
  FlatFigures() = default;
  ~FlatFigures() = default;

  // IModel interface implementation

  /**
   * @brief Adds a new figure/entity to the model at the specified position
   * @param position The position where the figure should be created (uses x and
   * y)
   * @note Creates a Triangle by default. For other figure types, use
   * createFigure()
   */
  void addFigure(const glm::vec3 &position) override {
    createFigure<Triangle>(position.x, position.y);
  }

  /**
   * @brief Removes a figure/entity from the model by its identifier
   * @param id The unique identifier of the figure to remove
   * @return true if the figure was found and removed, false otherwise
   */
  bool removeFigure(uint32_t id) override {
    spdlog::debug("[DEBUG] removeFigure called - id={}, Figures_.size()={}", id,
                  Figures_.size());
    if (id >= Figures_.size()) {
      spdlog::warn("[DEBUG] removeFigure - id {} >= size {}, returning false",
                   id, Figures_.size());
      return false;
    }
    auto figure = Figures_[id];
    if (figure) {
      spdlog::debug("[DEBUG] removeFigure - removing figure at index {} with "
                    "actual id={}",
                    id, figure->getId());
    } else {
      spdlog::warn("[DEBUG] removeFigure - Figures_[{}] is nullptr!", id);
    }
    Figures_.erase(Figures_.begin() + id);
    spdlog::debug("[DEBUG] removeFigure - after erase, Figures_.size()={}",
                  Figures_.size());
    return true;
  }

  /**
   * @brief Retrieves a figure/entity by its identifier
   * @param id The unique identifier of the figure to retrieve
   * @return Pointer to the figure if found, or nullptr if not found
   */
  std::shared_ptr<IFigure> getFigure(uint32_t id) override {
    return getFlatFigure(id);
  }

  /**
   * @brief Finds a figure near the specified coordinates
   * @param coords The coordinates to search around (using x and y components)
   * @param delta Maximum distance to consider a figure as "found"
   * @return Pointer to the found figure if any, or nullptr if none found
   */
  std::shared_ptr<IFigure> findFigureByCoords(const glm::vec3 &coords,
                                              float delta) override {
    return findFlatFigureByCoords(glm::vec2(coords.x, coords.y), delta);
  }

  /**
   * @brief Updates the position of an existing figure
   * @param id The unique identifier of the figure to update
   * @param newPosition The new position for the figure
   * @return true if the figure was found and updated, false otherwise
   */
  bool updateFigurePosition(uint32_t id,
                            const glm::vec3 &newPosition) override {
    auto figure = getFlatFigure(id);
    if (!figure) {
      return false;
    }
    figure->setPosition(newPosition);
    return true;
  }

  /**
   * @brief Gets the total number of figures in the model
   * @return The count of figures currently stored
   */
  size_t getFigureCount() const override { return Figures_.size(); }

  /**
   * @brief Clears all figures from the model
   */
  void clearFigures() override { Figures_.clear(); }

  /**
   * @brief Gets the camera interface for this model
   * @return Reference to the camera interface
   * @note Returns a reference to a simple wrapper around the internal camera
   */
  ICamera &getCamera() override {
    spdlog::debug("[DEBUG] getCamera() called - this={}, &camera_={}",
                  (void *)this, (void *)&camera_);
    static struct CameraWrapper : public ICamera {
      Camera_t *camera_;
      CameraWrapper(Camera_t *cam) : camera_(cam) {
        spdlog::debug("[DEBUG] CameraWrapper constructed - camera_={}",
                      (void *)camera_);
      }
      void setPosition(const glm::vec3 &position) override {
        spdlog::debug("[DEBUG] CameraWrapper::setPosition called - camera_={}",
                      (void *)camera_);
        camera_->position = position;
      }
      glm::vec3 getPosition() const override {
        spdlog::debug("[DEBUG] CameraWrapper::getPosition called - camera_={}",
                      (void *)camera_);
        return camera_->position;
      }
      void setTarget(const glm::vec3 &target) override {
        spdlog::debug("[DEBUG] CameraWrapper::setTarget called - camera_={}",
                      (void *)camera_);
        camera_->target = target;
      }
      glm::vec3 getTarget() const override {
        spdlog::debug("[DEBUG] CameraWrapper::getTarget called - camera_={}",
                      (void *)camera_);
        return camera_->target;
      }
      void setUp(const glm::vec3 &up) override {
        spdlog::debug("[DEBUG] CameraWrapper::setUp called - camera_={}",
                      (void *)camera_);
        camera_->up = up;
      }
      glm::vec3 getUp() const override {
        spdlog::debug("[DEBUG] CameraWrapper::getUp called - camera_={}",
                      (void *)camera_);
        return camera_->up;
      }
    } wrapper(&camera_);
    spdlog::debug("[DEBUG] getCamera() returning wrapper - wrapper.camera_={}",
                  (void *)wrapper.camera_);
    return wrapper;
  }

  /**
   * @brief Gets the camera interface for this model (const version)
   * @return Const reference to the camera interface
   */
  const ICamera &getCamera() const override {
    static struct CameraWrapper : public ICamera {
      const Camera_t *camera_;
      CameraWrapper(const Camera_t *cam) : camera_(cam) {}
      void setPosition(const glm::vec3 &position) override {
        const_cast<Camera_t *>(camera_)->position = position;
      }
      glm::vec3 getPosition() const override { return camera_->position; }
      void setTarget(const glm::vec3 &target) override {
        const_cast<Camera_t *>(camera_)->target = target;
      }
      glm::vec3 getTarget() const override { return camera_->target; }
      void setUp(const glm::vec3 &up) override {
        const_cast<Camera_t *>(camera_)->up = up;
      }
      glm::vec3 getUp() const override { return camera_->up; }
    } wrapper(&camera_);
    return wrapper;
  }

  /**
   * @brief Gets the model transformation matrix
   * @return The current model matrix
   */
  glm::mat4 getModelMatrix() const override { return model_; }

  /**
   * @brief Sets the model transformation matrix
   * @param matrix The new model matrix
   */
  void setModelMatrix(const glm::mat4 &matrix) override { model_ = matrix; }

  /**
   * @brief Gets the view transformation matrix
   * @return The current view matrix
   */
  glm::mat4 getViewMatrix() const override {
    return glm::lookAt(camera_.position, camera_.target, camera_.up);
  }

  /**
   * @brief Gets the projection transformation matrix
   * @return The current projection matrix
   */
  glm::mat4 getProjectionMatrix() const override { return projection_; }

  /**
   * @brief Sets the projection transformation matrix
   * @param matrix The new projection matrix
   */
  void setProjectionMatrix(const glm::mat4 &matrix) override {
    projection_ = matrix;
  }

  // Additional public methods for backward compatibility

  /**
   * @brief Creates a new figure of the specified type at the given position
   * @tparam T The figure type to create (e.g., Triangle, Quad, Circle)
   * @param x X coordinate for the figure's position
   * @param y Y coordinate for the figure's position
   * @param scribed Whether the figure is scribed (inscribed/circumscribed) or
   * not
   * @note The figure is created with default geometry and positioned at (x, y,
   * 0)
   */
  template <class T>
  void createFigure(float x, float y, isScribed scribed = isScribed::no) {
    auto figure = std::make_shared<Figure<T>>(x, y, 0.0f, scribed);
    spdlog::debug(
        "[DEBUG] createFigure - created figure with id={}, pushing to index {}",
        figure->getId(), Figures_.size());
    Figures_.push_back(figure);
    spdlog::debug("[DEBUG] createFigure - after push, Figures_.size()={}",
                  Figures_.size());
    if constexpr (std::is_same_v<T, Triangle>) {
      spdlog::info("Triangle in model");
      // Save all three points in one place here
    }
  }

  /**
   * @brief Adds a fully-constructed figure to the model
   * @param figure Shared pointer to the figure to add
   * @note This method allows adding figures with custom geometry
   */
  void addFigurePtr(std::shared_ptr<IFigure> figure) {
    if (figure) {
      spdlog::debug(
          "[DEBUG] addFigurePtr - adding figure with id={}, size before={}",
          figure->getId(), Figures_.size());
      Figures_.push_back(figure);
      spdlog::debug("[DEBUG] addFigurePtr - size after={}", Figures_.size());
    }
  }

  /**
   * @brief Creates a memento snapshot of the current model state
   * @return Unique pointer to the created memento
   * @note Implements the Memento pattern for state persistence
   */
  std::unique_ptr<Memento> createMemento() {
    return std::make_unique<Memento>(Figures_);
  }

  /**
   * @brief Restores the model state from a memento
   * @param mem The memento to restore from
   */
  void restoreFromMemento(const Memento &mem) { Figures_ = mem.Figures_; }
};

} // namespace model
