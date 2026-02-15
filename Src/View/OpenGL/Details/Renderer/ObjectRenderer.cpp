#include "ObjectRenderer.hpp"

#include <GLProgram.hpp>
#include <Shader.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace view {

namespace {
/// Vertex shader source for 3D object rendering
constexpr const char *VERTEX_SHADER_SOURCE = R"(
  #version 330 core
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 aNormal;
  layout (location = 2) in vec2 aTexCoord;
  layout (location = 3) in vec3 aTangent;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 projection;

  out vec3 FragPos;
  out vec3 Normal;

  void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
  }
)";

/// Fragment shader source for 3D object rendering
constexpr const char *FRAGMENT_SHADER_SOURCE = R"(
  #version 330 core
  in vec3 FragPos;
  in vec3 Normal;

  out vec4 FragColor;

  uniform vec3 lightPos;
  uniform vec3 viewPos;

  void main() {
    // Simple lighting
    vec3 objectColor = vec3(0.6, 0.6, 0.6);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
  }
)";
} // namespace

ObjectRenderer::ObjectRenderer()
    : shaderProgram_(std::make_unique<GLProgram>()), modelMatrixLocation_(-1),
      viewMatrixLocation_(-1), projectionMatrixLocation_(-1),
      wireframeMode_(false), visible_(true) {
  initialize();
}

ObjectRenderer::~ObjectRenderer() { cleanup(); }

ObjectRenderer::ObjectRenderer(ObjectRenderer &&other) noexcept
    : models_(std::move(other.models_)),
      shaderProgram_(std::move(other.shaderProgram_)),
      modelMatrixLocation_(other.modelMatrixLocation_),
      viewMatrixLocation_(other.viewMatrixLocation_),
      projectionMatrixLocation_(other.projectionMatrixLocation_),
      wireframeMode_(other.wireframeMode_), visible_(other.visible_) {
  other.modelMatrixLocation_ = -1;
  other.viewMatrixLocation_ = -1;
  other.projectionMatrixLocation_ = -1;
}

ObjectRenderer &ObjectRenderer::operator=(ObjectRenderer &&other) noexcept {
  if (this != &other) {
    cleanup();
    models_ = std::move(other.models_);
    shaderProgram_ = std::move(other.shaderProgram_);
    modelMatrixLocation_ = other.modelMatrixLocation_;
    viewMatrixLocation_ = other.viewMatrixLocation_;
    projectionMatrixLocation_ = other.projectionMatrixLocation_;
    wireframeMode_ = other.wireframeMode_;
    visible_ = other.visible_;
    other.modelMatrixLocation_ = -1;
    other.viewMatrixLocation_ = -1;
    other.projectionMatrixLocation_ = -1;
  }
  return *this;
}

Model3D *ObjectRenderer::addModel(std::unique_ptr<Model3D> model) {
  if (!model) {
    return nullptr;
  }
  Model3D *ptr = model.get();
  models_.push_back(std::move(model));
  return ptr;
}

void ObjectRenderer::removeModel(const Model3D *model) {
  models_.erase(std::remove_if(models_.begin(), models_.end(),
                               [model](const std::unique_ptr<Model3D> &m) {
                                 return m.get() == model;
                               }),
                models_.end());
}

void ObjectRenderer::clearModels() { models_.clear(); }

std::vector<const Model3D *> ObjectRenderer::getModels() const {
  std::vector<const Model3D *> result;
  result.reserve(models_.size());
  for (const auto &model : models_) {
    result.push_back(model.get());
  }
  return result;
}

void ObjectRenderer::render(const Camera3D &camera, float aspectRatio) {
  if (!visible_) {
    return;
  }

  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);
  render(view, projection);
}

void ObjectRenderer::render(const glm::mat4 &view,
                            const glm::mat4 &projection) {
  if (!visible_ || models_.empty()) {
    return;
  }

  // Enable depth testing for 3D rendering
  glEnable(GL_DEPTH_TEST);

  // Set polygon mode based on wireframe setting
  if (wireframeMode_) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Use shader program
  shaderProgram_->useProgram();

  // Set view and projection matrices
  if (viewMatrixLocation_ >= 0) {
    glUniformMatrix4fv(viewMatrixLocation_, 1, GL_FALSE, &view[0][0]);
  }
  if (projectionMatrixLocation_ >= 0) {
    glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE,
                       &projection[0][0]);
  }

  // Render all visible models
  for (const auto &model : models_) {
    if (model && model->isVisible()) {
      // Set model matrix
      glm::mat4 modelMatrix = model->getModelMatrix();
      if (modelMatrixLocation_ >= 0) {
        glUniformMatrix4fv(modelMatrixLocation_, 1, GL_FALSE,
                           &modelMatrix[0][0]);
      }
      model->render();
    }
  }

  // Restore polygon mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ObjectRenderer::setWireframeMode(bool wireframe) {
  wireframeMode_ = wireframe;
}

void ObjectRenderer::initialize() { createShaderProgram(); }

void ObjectRenderer::cleanup() {
  models_.clear();
  shaderProgram_.reset();
  modelMatrixLocation_ = -1;
  viewMatrixLocation_ = -1;
  projectionMatrixLocation_ = -1;
}

void ObjectRenderer::createShaderProgram() {
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
  auto modelLoc = shaderProgram_->getUniformLocation("model");
  auto viewLoc = shaderProgram_->getUniformLocation("view");
  auto projLoc = shaderProgram_->getUniformLocation("projection");

  if (!viewLoc || !projLoc) {
    throw std::runtime_error(
        "Failed to get uniform locations for ObjectRenderer shader");
  }

  modelMatrixLocation_ = modelLoc.value_or(-1);
  viewMatrixLocation_ = *viewLoc;
  projectionMatrixLocation_ = *projLoc;
}

} // namespace view
