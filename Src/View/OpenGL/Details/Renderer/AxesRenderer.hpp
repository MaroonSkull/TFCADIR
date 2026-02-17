#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>

class GLProgram;

namespace view {

/**
 * @brief Renders 3D coordinate axes for visualization
 * @details Displays X (red), Y (green), Z (blue) axes originating from origin.
 *          Used for 3D scene orientation reference.
 */
class AxesRenderer {
public:
  /**
   * @brief Constructs an AxesRenderer with default settings
   * @details Initializes axis length to 1.0, width to 2.0, and visible by
   * default.
   */
  AxesRenderer();

  /**
   * @brief Destructor - cleans up OpenGL resources
   */
  ~AxesRenderer();

  // Prevent copying
  AxesRenderer(const AxesRenderer &) = delete;
  AxesRenderer &operator=(const AxesRenderer &) = delete;

  // Allow moving
  AxesRenderer(AxesRenderer &&) noexcept;
  AxesRenderer &operator=(AxesRenderer &&) noexcept;

  /**
   * @brief Renders the coordinate axes
   * @param view The view matrix from the camera
   * @param projection The projection matrix from the camera
   * @details Binds shader, sets uniforms, and draws the three axis lines.
   */
  void render(const glm::mat4 &view, const glm::mat4 &projection);

  /**
   * @brief Sets the length of the axes
   * @param length The new axis length in world units
   */
  void setAxisLength(float length);

  /**
   * @brief Sets the line width of the axes
   * @param width The line width in pixels
   */
  void setAxisWidth(float width);

  /**
   * @brief Sets the visibility of the axes
   * @param visible True to show axes, false to hide
   */
  void setVisible(bool visible);

  /**
   * @brief Gets the current axis length
   * @return The axis length in world units
   */
  [[nodiscard]] float getAxisLength() const { return axisLength_; }

  /**
   * @brief Gets the current axis width
   * @return The line width in pixels
   */
  [[nodiscard]] float getAxisWidth() const { return axisWidth_; }

  /**
   * @brief Checks if axes are visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isVisible() const { return visible_; }

private:
  /// Axis length in world units
  float axisLength_;

  /// Line width in pixels
  float axisWidth_;

  /// Visibility flag
  bool visible_;

  /// Vertex Array Object for axis geometry
  GLuint vao_;

  /// Vertex Buffer Object for axis vertices
  GLuint vbo_;

  /// Shader program for rendering
  std::unique_ptr<GLProgram> shaderProgram_;

  /// Uniform location for view matrix
  GLint viewMatrixLocation_;

  /// Uniform location for projection matrix
  GLint projectionMatrixLocation_;

  /**
   * @brief Initializes OpenGL resources (VAO, VBO, shaders)
   */
  void initialize();

  /**
   * @brief Cleans up OpenGL resources
   */
  void cleanup();

  /**
   * @brief Creates the shader program for axis rendering
   */
  void createShaderProgram();

  /**
   * @brief Generates vertex data for the axes
   * @param length The axis length
   * @return Vertex data array (position x,y,z, color r,g,b for each vertex)
   */
  [[nodiscard]] std::array<float, 36> generateVertexData(float length) const;
};

} // namespace view
