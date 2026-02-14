#include "AxesRenderer.hpp"

#include <GLProgram.hpp>
#include <Shader.hpp>

#include <array>
#include <stdexcept>

namespace view {

namespace {
/// Vertex shader source for axis rendering
constexpr const char *VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vertexColor = aColor;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

/// Fragment shader source for axis rendering
constexpr const char *FRAGMENT_SHADER_SOURCE = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";
} // namespace

AxesRenderer::AxesRenderer()
    : axisLength_(1.0f), axisWidth_(2.0f), visible_(true), vao_(0), vbo_(0),
      shaderProgram_(nullptr), viewMatrixLocation_(-1),
      projectionMatrixLocation_(-1) {
  initialize();
}

AxesRenderer::~AxesRenderer() { cleanup(); }

AxesRenderer::AxesRenderer(AxesRenderer &&other) noexcept
    : axisLength_(other.axisLength_), axisWidth_(other.axisWidth_),
      visible_(other.visible_), vao_(other.vao_), vbo_(other.vbo_),
      shaderProgram_(std::move(other.shaderProgram_)),
      viewMatrixLocation_(other.viewMatrixLocation_),
      projectionMatrixLocation_(other.projectionMatrixLocation_) {
  other.vao_ = 0;
  other.vbo_ = 0;
  other.viewMatrixLocation_ = -1;
  other.projectionMatrixLocation_ = -1;
}

AxesRenderer &AxesRenderer::operator=(AxesRenderer &&other) noexcept {
  if (this != &other) {
    cleanup();

    axisLength_ = other.axisLength_;
    axisWidth_ = other.axisWidth_;
    visible_ = other.visible_;
    vao_ = other.vao_;
    vbo_ = other.vbo_;
    shaderProgram_ = std::move(other.shaderProgram_);
    viewMatrixLocation_ = other.viewMatrixLocation_;
    projectionMatrixLocation_ = other.projectionMatrixLocation_;

    other.vao_ = 0;
    other.vbo_ = 0;
    other.viewMatrixLocation_ = -1;
    other.projectionMatrixLocation_ = -1;
  }
  return *this;
}

void AxesRenderer::render(const glm::mat4 &view, const glm::mat4 &projection) {
  if (!visible_ || !shaderProgram_) {
    return;
  }

  // Store current line width
  GLfloat previousLineWidth;
  glGetFloatv(GL_LINE_WIDTH, &previousLineWidth);

  // Set line width
  glLineWidth(axisWidth_);

  // Use shader program
  shaderProgram_->useProgram();

  // Set uniforms
  glUniformMatrix4fv(viewMatrixLocation_, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, &projection[0][0]);

  // Bind VAO and draw
  glBindVertexArray(vao_);
  glDrawArrays(GL_LINES, 0, 6);
  glBindVertexArray(0);

  // Restore previous line width
  glLineWidth(previousLineWidth);
}

void AxesRenderer::setAxisLength(float length) {
  if (length > 0.0f) {
    axisLength_ = length;
    // Update vertex data
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    auto vertexData = generateVertexData(axisLength_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float),
                    vertexData.data());
    glBindVertexArray(0);
  }
}

void AxesRenderer::setAxisWidth(float width) {
  axisWidth_ = width > 0.0f ? width : 1.0f;
}

void AxesRenderer::setVisible(bool visible) { visible_ = visible; }

void AxesRenderer::initialize() {
  // Create shader program
  createShaderProgram();

  // Generate vertex data
  auto vertexData = generateVertexData(axisLength_);

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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  // Color attribute (location = 1)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Unbind VAO
  glBindVertexArray(0);
}

void AxesRenderer::cleanup() {
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

void AxesRenderer::createShaderProgram() {
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

  if (!viewLoc || !projLoc) {
    throw std::runtime_error(
        "Failed to get uniform locations for AxesRenderer shader");
  }

  viewMatrixLocation_ = *viewLoc;
  projectionMatrixLocation_ = *projLoc;
}

std::array<float, 36> AxesRenderer::generateVertexData(float length) const {
  // 6 vertices (2 per axis), each with position (3) and color (3) = 6 floats
  // per vertex Total: 6 vertices * 6 floats = 36 floats
  //
  // X axis: Red (1.0, 0.0, 0.0)
  // Y axis: Green (0.0, 1.0, 0.0)
  // Z axis: Blue (0.0, 0.0, 1.0)

  return {// X axis (red)
          // Origin vertex
          0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
          // X endpoint vertex
          length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

          // Y axis (green)
          // Origin vertex
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
          // Y endpoint vertex
          0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f,

          // Z axis (blue)
          // Origin vertex
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
          // Z endpoint vertex
          0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f};
}

} // namespace view
