#pragma once

#include "../Camera/Camera3D.hpp"
#include "Model3D.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class GLProgram;

namespace view {

/**
 * @brief Main object rendering system for 3D visualization
 * @details Manages a collection of 3D models and renders them with proper
 *          transformations. Integrates with the camera system for view and
 *          projection matrices.
 */
class ObjectRenderer {
public:
  /**
   * @brief Constructs an ObjectRenderer with default settings
   */
  ObjectRenderer();

  /**
   * @brief Destructor - cleans up OpenGL resources
   */
  ~ObjectRenderer();

  // Prevent copying
  ObjectRenderer(const ObjectRenderer &) = delete;
  ObjectRenderer &operator=(const ObjectRenderer &) = delete;

  // Allow moving
  ObjectRenderer(ObjectRenderer &&) noexcept;
  ObjectRenderer &operator=(ObjectRenderer &&) noexcept;

  // ==========================================================================
  // Model Management
  // ==========================================================================

  /**
   * @brief Adds a model to the renderer
   * @param model The model to add
   * @return Pointer to the added model for further manipulation
   */
  Model3D *addModel(std::unique_ptr<Model3D> model);

  /**
   * @brief Removes a model from the renderer
   * @param model Pointer to the model to remove
   */
  void removeModel(const Model3D *model);

  /**
   * @brief Removes all models from the renderer
   */
  void clearModels();

  /**
   * @brief Gets the number of models in the renderer
   * @return Model count
   */
  [[nodiscard]] size_t getModelCount() const { return models_.size(); }

  /**
   * @brief Gets all models in the renderer
   * @return Vector of model pointers
   */
  [[nodiscard]] std::vector<const Model3D *> getModels() const;

  // ==========================================================================
  // Rendering
  // ==========================================================================

  /**
   * @brief Renders all models with the given camera
   * @param camera The camera to use for view and projection matrices
   * @param aspectRatio The viewport aspect ratio (width / height)
   * @details Binds shader, sets camera uniforms, and renders all visible
   *          models with their transformations.
   */
  void render(const Camera3D &camera, float aspectRatio);

  /**
   * @brief Renders all models with explicit matrices
   * @param view The view matrix
   * @param projection The projection matrix
   * @details Useful for custom camera setups or testing.
   */
  void render(const glm::mat4 &view, const glm::mat4 &projection);

  // ==========================================================================
  // Configuration
  // ==========================================================================

  /**
   * @brief Sets whether to render in wireframe mode
   * @param wireframe True for wireframe, false for solid
   */
  void setWireframeMode(bool wireframe);

  /**
   * @brief Checks if wireframe mode is enabled
   * @return True if wireframe mode is active
   */
  [[nodiscard]] bool isWireframeMode() const { return wireframeMode_; }

  /**
   * @brief Sets the visibility of the renderer
   * @param visible True to enable rendering, false to disable
   */
  void setVisible(bool visible) { visible_ = visible; }

  /**
   * @brief Checks if the renderer is visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isVisible() const { return visible_; }

private:
  /// Collection of models to render
  std::vector<std::unique_ptr<Model3D>> models_;

  /// Shader program for rendering
  std::unique_ptr<GLProgram> shaderProgram_;

  /// Uniform location for model matrix
  GLint modelMatrixLocation_;

  /// Uniform location for view matrix
  GLint viewMatrixLocation_;

  /// Uniform location for projection matrix
  GLint projectionMatrixLocation_;

  /// Wireframe rendering mode
  bool wireframeMode_;

  /// Renderer visibility
  bool visible_;

  /**
   * @brief Initializes OpenGL resources and shader program
   */
  void initialize();

  /**
   * @brief Cleans up OpenGL resources
   */
  void cleanup();

  /**
   * @brief Creates the shader program for object rendering
   */
  void createShaderProgram();

  /**
   * @brief Sets up shader uniforms
   */
  void setupUniforms();
};

} // namespace view
