#include "GridRenderer.hpp"

#include <GLProgram.hpp>
#include <Shader.hpp>

#include <stdexcept>

namespace view {

namespace {
/// Vertex shader source for grid rendering
constexpr const char *VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

/// Fragment shader source for grid rendering
constexpr const char *FRAGMENT_SHADER_SOURCE = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 gridColor;

void main() {
    FragColor = vec4(gridColor, 1.0);
}
)";
} // namespace

GridRenderer::GridRenderer()
    : gridSize_(20), gridSpacing_(1.0f), gridColor_(0.5f, 0.5f, 0.5f),
      visible_(true), vao_(0), vbo_(0), shaderProgram_(nullptr),
      viewMatrixLocation_(-1), projectionMatrixLocation_(-1),
      colorLocation_(-1), vertexCount_(0) {
  initialize();
}

GridRenderer::~GridRenderer() { cleanup(); }

GridRenderer::GridRenderer(GridRenderer &&other) noexcept
    : gridSize_(other.gridSize_), gridSpacing_(other.gridSpacing_),
      gridColor_(other.gridColor_), visible_(other.visible_), vao_(other.vao_),
      vbo_(other.vbo_), shaderProgram_(std::move(other.shaderProgram_)),
      viewMatrixLocation_(other.viewMatrixLocation_),
      projectionMatrixLocation_(other.projectionMatrixLocation_),
      colorLocation_(other.colorLocation_), vertexCount_(other.vertexCount_) {
  other.vao_ = 0;
  other.vbo_ = 0;
  other.viewMatrixLocation_ = -1;
  other.projectionMatrixLocation_ = -1;
  other.colorLocation_ = -1;
  other.vertexCount_ = 0;
}

GridRenderer &GridRenderer::operator=(GridRenderer &&other) noexcept {
  if (this != &other) {
    cleanup();

    gridSize_ = other.gridSize_;
    gridSpacing_ = other.gridSpacing_;
    gridColor_ = other.gridColor_;
    visible_ = other.visible_;
    vao_ = other.vao_;
    vbo_ = other.vbo_;
    shaderProgram_ = std::move(other.shaderProgram_);
    viewMatrixLocation_ = other.viewMatrixLocation_;
    projectionMatrixLocation_ = other.projectionMatrixLocation_;
    colorLocation_ = other.colorLocation_;
    vertexCount_ = other.vertexCount_;

    other.vao_ = 0;
    other.vbo_ = 0;
    other.viewMatrixLocation_ = -1;
    other.projectionMatrixLocation_ = -1;
    other.colorLocation_ = -1;
    other.vertexCount_ = 0;
  }
  return *this;
}

void GridRenderer::render(const glm::mat4 &view, const glm::mat4 &projection) {
  if (!visible_ || !shaderProgram_) {
    return;
  }

  // Use shader program
  shaderProgram_->useProgram();

  // Set uniforms
  glUniformMatrix4fv(viewMatrixLocation_, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, &projection[0][0]);
  glUniform3fv(colorLocation_, 1, &gridColor_[0]);

  // Bind VAO and draw
  glBindVertexArray(vao_);
  glDrawArrays(GL_LINES, 0, vertexCount_);
  glBindVertexArray(0);
}

void GridRenderer::setGridSize(int size) {
  if (size > 0 && size != gridSize_) {
    gridSize_ = size;
    // Regenerate vertex data
    auto vertexData = generateVertexData();
    vertexCount_ = static_cast<GLsizei>(vertexData.size() / 3);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                 vertexData.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
  }
}

void GridRenderer::setGridSpacing(float spacing) {
  if (spacing > 0.0f && spacing != gridSpacing_) {
    gridSpacing_ = spacing;
    // Regenerate vertex data
    auto vertexData = generateVertexData();
    vertexCount_ = static_cast<GLsizei>(vertexData.size() / 3);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                 vertexData.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
  }
}

void GridRenderer::setGridColor(const glm::vec3 &color) { gridColor_ = color; }

void GridRenderer::setVisible(bool visible) { visible_ = visible; }

void GridRenderer::initialize() {
  // Create shader program
  createShaderProgram();

  // Generate vertex data
  auto vertexData = generateVertexData();
  vertexCount_ = static_cast<GLsizei>(vertexData.size() / 3);

  // Create VAO and VBO
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  // Bind VAO
  glBindVertexArray(vao_);

  // Bind and set VBO data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
               vertexData.data(), GL_DYNAMIC_DRAW);

  // Position attribute (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  // Unbind VAO
  glBindVertexArray(0);
}

void GridRenderer::cleanup() {
  if (vbo_ != 0) {
    glDeleteBuffers(1, &vbo_);
    vbo_ = 0;
  }
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
  }
  shaderProgram_.reset();
}

void GridRenderer::createShaderProgram() {
  // Create shader objects using Resource class
  Resource vertexSource(VERTEX_SHADER_SOURCE,
                        std::char_traits<char>::length(VERTEX_SHADER_SOURCE));
  Resource fragmentSource(
      FRAGMENT_SHADER_SOURCE,
      std::char_traits<char>::length(FRAGMENT_SHADER_SOURCE));

  // Create shaders
  Shader vertexShader(vertexSource, Shader::type::Vertex);
  Shader fragmentShader(fragmentSource, Shader::type::Fragment);

  // Create program and attach shaders
  shaderProgram_ = std::make_unique<GLProgram>();
  shaderProgram_->attachShader(&vertexShader);
  shaderProgram_->attachShader(&fragmentShader);
  shaderProgram_->linkProgram();

  // Get uniform locations
  auto viewLoc = shaderProgram_->getUniformLocation("view");
  auto projLoc = shaderProgram_->getUniformLocation("projection");
  auto colorLoc = shaderProgram_->getUniformLocation("gridColor");

  if (!viewLoc || !projLoc || !colorLoc) {
    throw std::runtime_error(
        "Failed to get uniform locations for GridRenderer shader");
  }

  viewMatrixLocation_ = *viewLoc;
  projectionMatrixLocation_ = *projLoc;
  colorLocation_ = *colorLoc;
}

std::vector<float> GridRenderer::generateVertexData() const {
  std::vector<float> vertices;

  // Calculate number of lines needed per direction
  // Lines extend from -gridSize_ to +gridSize_ with spacing gridSpacing_
  int numLinesPerDirection =
      static_cast<int>(gridSize_ / gridSpacing_) * 2 + 1;

  // Reserve space: each line has 2 vertices, each vertex has 3 floats (x, y, z)
  // Two sets of lines: parallel to X axis and parallel to Z axis
  vertices.reserve(static_cast<size_t>(numLinesPerDirection) * 2 * 3 * 2);

  float halfSize = static_cast<float>(gridSize_);

  // Generate lines parallel to X axis (varying Z)
  for (int i = 0; i < numLinesPerDirection; ++i) {
    float z = -halfSize + static_cast<float>(i) * gridSpacing_;
    // Clamp to grid extent to handle floating point precision issues
    if (z > halfSize) {
      z = halfSize;
    }
    // Line from (-halfSize, 0, z) to (halfSize, 0, z)
    vertices.push_back(-halfSize); // x1
    vertices.push_back(0.0f);      // y1
    vertices.push_back(z);         // z1
    vertices.push_back(halfSize);  // x2
    vertices.push_back(0.0f);      // y2
    vertices.push_back(z);         // z2
  }

  // Generate lines parallel to Z axis (varying X)
  for (int i = 0; i < numLinesPerDirection; ++i) {
    float x = -halfSize + static_cast<float>(i) * gridSpacing_;
    // Clamp to grid extent to handle floating point precision issues
    if (x > halfSize) {
      x = halfSize;
    }
    // Line from (x, 0, -halfSize) to (x, 0, halfSize)
    vertices.push_back(x);         // x1
    vertices.push_back(0.0f);      // y1
    vertices.push_back(-halfSize); // z1
    vertices.push_back(x);         // x2
    vertices.push_back(0.0f);      // y2
    vertices.push_back(halfSize);  // z2
  }

  return vertices;
}

} // namespace view
