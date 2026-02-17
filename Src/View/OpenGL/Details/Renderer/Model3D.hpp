#pragma once

#include "Mesh3D.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>

namespace view {

/**
 * @brief 3D model container managing multiple meshes and transformations
 * @details A Model3D represents a complete 3D object composed of one or more
 *          meshes. It manages model transformations (position, rotation, scale)
 *          and provides rendering functionality.
 */
class Model3D {
public:
  /**
   * @brief Constructs an empty model at origin with default scale
   */
  Model3D();

  /**
   * @brief Constructs a model with a single mesh
   * @param mesh The mesh to add to the model
   */
  explicit Model3D(std::unique_ptr<Mesh3D> mesh);

  /**
   * @brief Destructor
   */
  ~Model3D() = default;

  // Prevent copying
  Model3D(const Model3D &) = delete;
  Model3D &operator=(const Model3D &) = delete;

  // Allow moving
  Model3D(Model3D &&) noexcept = default;
  Model3D &operator=(Model3D &&) noexcept = default;

  // ==========================================================================
  // Mesh Management
  // ==========================================================================

  /**
   * @brief Adds a mesh to the model
   * @param mesh The mesh to add
   */
  void addMesh(std::unique_ptr<Mesh3D> mesh);

  /**
   * @brief Gets the number of meshes in the model
   * @return Mesh count
   */
  [[nodiscard]] size_t getMeshCount() const { return meshes_.size(); }

  /**
   * @brief Checks if the model has any meshes
   * @return True if model has at least one mesh
   */
  [[nodiscard]] bool hasMeshes() const { return !meshes_.empty(); }

  // ==========================================================================
  // Transformation Methods
  // ==========================================================================

  /**
   * @brief Sets the model position in world space
   * @param position The new position
   */
  void setPosition(const glm::vec3 &position);

  /**
   * @brief Sets the model rotation (Euler angles in radians)
   * @param rotation Rotation angles around X, Y, Z axes
   */
  void setRotation(const glm::vec3 &rotation);

  /**
   * @brief Sets the model scale
   * @param scale Uniform scale factor
   */
  void setScale(float scale);

  /**
   * @brief Sets the model scale (non-uniform)
   * @param scale Scale factors for X, Y, Z axes
   */
  void setScale(const glm::vec3 &scale);

  /**
   * @brief Translates the model by the given offset
   * @param offset Translation offset
   */
  void translate(const glm::vec3 &offset);

  /**
   * @brief Rotates the model by the given angles
   * @param angles Rotation angles in radians (X, Y, Z)
   */
  void rotate(const glm::vec3 &angles);

  /**
   * @brief Resets all transformations to default
   */
  void resetTransform();

  // ==========================================================================
  // Transformation Getters
  // ==========================================================================

  /**
   * @brief Gets the model position
   * @return Current position in world space
   */
  [[nodiscard]] const glm::vec3 &getPosition() const { return position_; }

  /**
   * @brief Gets the model rotation (Euler angles in radians)
   * @return Rotation angles around X, Y, Z axes
   */
  [[nodiscard]] const glm::vec3 &getRotation() const { return rotation_; }

  /**
   * @brief Gets the model scale
   * @return Scale factors for X, Y, Z axes
   */
  [[nodiscard]] const glm::vec3 &getScale() const { return scale_; }

  /**
   * @brief Gets the model transformation matrix
   * @return Combined translation, rotation, and scale matrix
   */
  [[nodiscard]] glm::mat4 getModelMatrix() const;

  // ==========================================================================
  // Rendering
  // ==========================================================================

  /**
   * @brief Renders all meshes in the model
   * @details Binds each mesh and draws it. The model matrix should be set
   *          in the shader before calling this method.
   */
  void render() const;

  /**
   * @brief Checks if the model is visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isVisible() const { return visible_; }

  /**
   * @brief Sets the model visibility
   * @param visible True to show, false to hide
   */
  void setVisible(bool visible) { visible_ = visible; }

private:
  /// Meshes that make up this model
  std::vector<std::unique_ptr<Mesh3D>> meshes_;

  /// Position in world space
  glm::vec3 position_;

  /// Rotation angles in radians (X, Y, Z)
  glm::vec3 rotation_;

  /// Scale factors (X, Y, Z)
  glm::vec3 scale_;

  /// Visibility flag
  bool visible_;
};

} // namespace view
