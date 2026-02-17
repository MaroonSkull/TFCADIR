#pragma once

#include "Camera/Camera3D.hpp"
#include "Camera/OrbitControls.hpp"
#include "Renderer/AxesRenderer.hpp"
#include "Renderer/GridRenderer.hpp"
#include "Renderer/ObjectRenderer.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>

namespace view {

/**
 * @brief Unified 3D rendering pipeline integrating all rendering components
 * @details Manages camera, grid, axes, and object rendering systems.
 *          Provides a simple API for rendering the complete 3D world.
 */
class RenderingPipeline3D {
public:
  /**
   * @brief Constructs a rendering pipeline with default settings
   */
  RenderingPipeline3D();

  /**
   * @brief Destructor - cleans up all resources
   */
  ~RenderingPipeline3D();

  // Prevent copying
  RenderingPipeline3D(const RenderingPipeline3D &) = delete;
  RenderingPipeline3D &operator=(const RenderingPipeline3D &) = delete;

  // Allow moving (note: move assignment is deleted due to OrbitControls
  // containing a reference member)
  RenderingPipeline3D(RenderingPipeline3D &&) noexcept;
  RenderingPipeline3D &operator=(RenderingPipeline3D &&) = delete;

  // ==========================================================================
  // Initialization and Cleanup
  // ==========================================================================

  /**
   * @brief Initializes the rendering pipeline
   * @param viewportWidth Initial viewport width in pixels
   * @param viewportHeight Initial viewport height in pixels
   * @return True if initialization succeeded, false otherwise
   */
  bool initialize(int viewportWidth, int viewportHeight);

  /**
   * @brief Cleans up all rendering resources
   */
  void cleanup();

  /**
   * @brief Checks if the pipeline is initialized
   * @return True if initialized, false otherwise
   */
  [[nodiscard]] bool isInitialized() const { return initialized_; }

  // ==========================================================================
  // Rendering
  // ==========================================================================

  /**
   * @brief Renders the complete 3D world
   * @details Renders in order: grid, axes, objects.
   *          Uses the current camera for view and projection matrices.
   */
  void render();

  /**
   * @brief Called when viewport size changes
   * @param width New viewport width in pixels
   * @param height New viewport height in pixels
   */
  void onViewportResize(int width, int height);

  // ==========================================================================
  // Camera Access
  // ==========================================================================

  /**
   * @brief Gets the camera instance
   * @return Reference to the camera
   */
  [[nodiscard]] Camera3D &getCamera() { return camera_; }

  /**
   * @brief Gets the camera instance (const version)
   * @return Const reference to the camera
   */
  [[nodiscard]] const Camera3D &getCamera() const { return camera_; }

  /**
   * @brief Gets the orbit controls
   * @return Reference to the orbit controls
   */
  [[nodiscard]] OrbitControls &getOrbitControls() { return orbitControls_; }

  /**
   * @brief Gets the orbit controls (const version)
   * @return Const reference to the orbit controls
   */
  [[nodiscard]] const OrbitControls &getOrbitControls() const {
    return orbitControls_;
  }

  // ==========================================================================
  // Renderer Access
  // ==========================================================================

  /**
   * @brief Gets the grid renderer
   * @return Reference to the grid renderer
   */
  [[nodiscard]] GridRenderer &getGridRenderer() { return gridRenderer_; }

  /**
   * @brief Gets the grid renderer (const version)
   * @return Const reference to the grid renderer
   */
  [[nodiscard]] const GridRenderer &getGridRenderer() const {
    return gridRenderer_;
  }

  /**
   * @brief Gets the axes renderer
   * @return Reference to the axes renderer
   */
  [[nodiscard]] AxesRenderer &getAxesRenderer() { return axesRenderer_; }

  /**
   * @brief Gets the axes renderer (const version)
   * @return Const reference to the axes renderer
   */
  [[nodiscard]] const AxesRenderer &getAxesRenderer() const {
    return axesRenderer_;
  }

  /**
   * @brief Gets the object renderer
   * @return Reference to the object renderer
   */
  [[nodiscard]] ObjectRenderer &getObjectRenderer() { return objectRenderer_; }

  /**
   * @brief Gets the object renderer (const version)
   * @return Const reference to the object renderer
   */
  [[nodiscard]] const ObjectRenderer &getObjectRenderer() const {
    return objectRenderer_;
  }

  // ==========================================================================
  // Background Configuration
  // ==========================================================================

  /**
   * @brief Sets the background clear color
   * @param color RGB clear color
   */
  void setBackgroundColor(const glm::vec3 &color);

  /**
   * @brief Gets the background clear color
   * @return Current clear color
   */
  [[nodiscard]] glm::vec3 getBackgroundColor() const {
    return backgroundColor_;
  }

  // ==========================================================================
  // Visibility Controls
  // ==========================================================================

  /**
   * @brief Sets grid visibility
   * @param visible True to show grid, false to hide
   */
  void setGridVisible(bool visible);

  /**
   * @brief Sets axes visibility
   * @param visible True to show axes, false to hide
   */
  void setAxesVisible(bool visible);

  /**
   * @brief Checks if grid is visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isGridVisible() const;

  /**
   * @brief Checks if axes are visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isAxesVisible() const;

private:
  /// Camera for view and projection
  Camera3D camera_;

  /// Orbit controls for camera manipulation
  OrbitControls orbitControls_;

  /// Grid renderer
  GridRenderer gridRenderer_;

  /// Axes renderer
  AxesRenderer axesRenderer_;

  /// Object renderer
  ObjectRenderer objectRenderer_;

  /// Background clear color
  glm::vec3 backgroundColor_;

  /// Current viewport width
  int viewportWidth_;

  /// Current viewport height
  int viewportHeight_;

  /// Initialization flag
  bool initialized_;

  /**
   * @brief Sets up OpenGL state for rendering
   */
  void setupGLState();

  /**
   * @brief Clears the screen with background color
   */
  void clearScreen();
};

} // namespace view
