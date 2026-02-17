#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <vector>

namespace view {

/**
 * @brief Vertex data structure for 3D meshes
 * @details Contains position, normal, texture coordinate, and tangent data
 *          for a single vertex in a 3D mesh.
 */
struct Vertex3D {
  /// Position in world space
  glm::vec3 position{0.0f};

  /// Surface normal vector
  glm::vec3 normal{0.0f, 1.0f, 0.0f};

  /// Texture coordinates
  glm::vec2 texCoord{0.0f};

  /// Tangent vector for normal mapping
  glm::vec3 tangent{1.0f, 0.0f, 0.0f};
};

/**
 * @brief 3D mesh data structure with OpenGL buffer management
 * @details Stores vertex and index data for a 3D mesh and manages
 *          OpenGL VAO, VBO, and EBO with RAII pattern.
 */
class Mesh3D {
public:
  /**
   * @brief Constructs an empty mesh
   * @details Initializes OpenGL buffers but does not upload any data.
   */
  Mesh3D();

  /**
   * @brief Constructs a mesh with vertex and index data
   * @param vertices Vector of vertex data
   * @param indices Vector of index data for indexed drawing
   */
  Mesh3D(const std::vector<Vertex3D> &vertices,
         const std::vector<unsigned int> &indices);

  /**
   * @brief Destructor - cleans up OpenGL resources
   */
  ~Mesh3D();

  // Prevent copying
  Mesh3D(const Mesh3D &) = delete;
  Mesh3D &operator=(const Mesh3D &) = delete;

  // Allow moving
  Mesh3D(Mesh3D &&other) noexcept;
  Mesh3D &operator=(Mesh3D &&other) noexcept;

  /**
   * @brief Loads vertex and index data into OpenGL buffers
   * @param vertices Vector of vertex data
   * @param indices Vector of index data for indexed drawing
   * @details Clears existing data and uploads new data to GPU.
   */
  void loadData(const std::vector<Vertex3D> &vertices,
                const std::vector<unsigned int> &indices);

  /**
   * @brief Binds the mesh's VAO for rendering
   */
  void bind() const;

  /**
   * @brief Unbinds the mesh's VAO
   */
  void unbind() const;

  /**
   * @brief Draws the mesh using indexed drawing
   * @details Must be called after bind() and with appropriate shader bound.
   */
  void draw() const;

  /**
   * @brief Checks if mesh has valid data
   * @return True if mesh has vertex and index data
   */
  [[nodiscard]] bool isValid() const { return vao_ != 0 && indexCount_ > 0; }

  /**
   * @brief Gets the number of indices in the mesh
   * @return Index count for indexed drawing
   */
  [[nodiscard]] GLsizei getIndexCount() const { return indexCount_; }

  /**
   * @brief Gets the number of vertices in the mesh
   * @return Vertex count
   */
  [[nodiscard]] GLsizei getVertexCount() const { return vertexCount_; }

private:
  /// Vertex Array Object
  GLuint vao_;

  /// Vertex Buffer Object
  GLuint vbo_;

  /// Element Buffer Object
  GLuint ebo_;

  /// Number of indices for drawing
  GLsizei indexCount_;

  /// Number of vertices
  GLsizei vertexCount_;

  /**
   * @brief Initializes OpenGL buffers (VAO, VBO, EBO)
   */
  void initialize();

  /**
   * @brief Cleans up OpenGL resources
   */
  void cleanup();

  /**
   * @brief Sets up vertex attributes for the VAO
   */
  void setupVertexAttributes();
};

} // namespace view
