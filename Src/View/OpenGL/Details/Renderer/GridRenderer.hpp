#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class GLProgram;

namespace view {

/**
 * @brief Renders a 3D grid on the XZ plane for spatial reference
 * @details Displays a grid of lines on the XZ plane (y = 0) centered at the
 * origin. Used for 3D scene spatial orientation and scale reference.
 */
class GridRenderer {
public:
  /**
   * @brief Constructs a GridRenderer with default settings
   * @details Initializes grid size to 20 units, spacing to 1.0 units, gray
   * color, and visible by default.
   */
  GridRenderer();

  /**
   * @brief Destructor - cleans up OpenGL resources
   */
  ~GridRenderer();

  // Prevent copying
  GridRenderer(const GridRenderer &) = delete;
  GridRenderer &operator=(const GridRenderer &) = delete;

  // Allow moving
  GridRenderer(GridRenderer &&) noexcept;
  GridRenderer &operator=(GridRenderer &&) noexcept;

  /**
   * @brief Renders the grid
   * @param view The view matrix from the camera
   * @param projection The projection matrix from the camera
   * @details Binds shader, sets uniforms, and draws the grid lines.
   */
  void render(const glm::mat4 &view, const glm::mat4 &projection);

  /**
   * @brief Sets the grid size (half-extent from center)
   * @param size The grid half-size in world units (grid goes from -size to
   * +size)
   */
  void setGridSize(int size);

  /**
   * @brief Sets the spacing between grid lines
   * @param spacing The distance between adjacent grid lines in world units
   */
  void setGridSpacing(float spacing);

  /**
   * @brief Sets the grid line color
   * @param color The RGB color of the grid lines
   */
  void setGridColor(const glm::vec3 &color);

  /**
   * @brief Sets the visibility of the grid
   * @param visible True to show grid, false to hide
   */
  void setVisible(bool visible);

  /**
   * @brief Gets the current grid size
   * @return The grid half-size in world units
   */
  [[nodiscard]] int getGridSize() const { return gridSize_; }

  /**
   * @brief Gets the current grid spacing
   * @return The spacing between grid lines in world units
   */
  [[nodiscard]] float getGridSpacing() const { return gridSpacing_; }

  /**
   * @brief Gets the current grid color
   * @return The RGB color of the grid lines
   */
  [[nodiscard]] glm::vec3 getGridColor() const { return gridColor_; }

  /**
   * @brief Checks if grid is visible
   * @return True if visible, false otherwise
   */
  [[nodiscard]] bool isVisible() const { return visible_; }

private:
  /// Grid half-size in world units (grid extends from -size to +size)
  int gridSize_;

  /// Spacing between grid lines in world units
  float gridSpacing_;

  /// Grid line color
  glm::vec3 gridColor_;

  /// Visibility flag
  bool visible_;

  /// Vertex Array Object for grid geometry
  GLuint vao_;

  /// Vertex Buffer Object for grid vertices
  GLuint vbo_;

  /// Shader program for rendering
  std::unique_ptr<GLProgram> shaderProgram_;

  /// Uniform location for view matrix
  GLint viewMatrixLocation_;

  /// Uniform location for projection matrix
  GLint projectionMatrixLocation_;

  /// Uniform location for grid color
  GLint colorLocation_;

  /// Number of vertices to draw
  GLsizei vertexCount_;

  /**
   * @brief Initializes OpenGL resources (VAO, VBO, shaders)
   */
  void initialize();

  /**
   * @brief Cleans up OpenGL resources
   */
  void cleanup();

  /**
   * @brief Creates the shader program for grid rendering
   */
  void createShaderProgram();

  /**
   * @brief Generates vertex data for the grid
   * @return Vertex data array (position x,y,z for each vertex)
   */
  [[nodiscard]] std::vector<float> generateVertexData() const;
};

} // namespace view
