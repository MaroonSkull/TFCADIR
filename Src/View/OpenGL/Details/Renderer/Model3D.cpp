#include "Model3D.hpp"

namespace view {

Model3D::Model3D()
    : position_(0.0f), rotation_(0.0f), scale_(1.0f), visible_(true) {}

Model3D::Model3D(std::unique_ptr<Mesh3D> mesh)
    : position_(0.0f), rotation_(0.0f), scale_(1.0f), visible_(true) {
  addMesh(std::move(mesh));
}

void Model3D::addMesh(std::unique_ptr<Mesh3D> mesh) {
  if (mesh) {
    meshes_.push_back(std::move(mesh));
  }
}

void Model3D::setPosition(const glm::vec3 &position) { position_ = position; }

void Model3D::setRotation(const glm::vec3 &rotation) { rotation_ = rotation; }

void Model3D::setScale(float scale) { scale_ = glm::vec3(scale); }

void Model3D::setScale(const glm::vec3 &scale) { scale_ = scale; }

void Model3D::translate(const glm::vec3 &offset) { position_ += offset; }

void Model3D::rotate(const glm::vec3 &angles) { rotation_ += angles; }

void Model3D::resetTransform() {
  position_ = glm::vec3(0.0f);
  rotation_ = glm::vec3(0.0f);
  scale_ = glm::vec3(1.0f);
}

glm::mat4 Model3D::getModelMatrix() const {
  glm::mat4 model = glm::mat4(1.0f);

  // Translate
  model = glm::translate(model, position_);

  // Rotate (X, Y, Z order)
  model = glm::rotate(model, rotation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, rotation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, rotation_.z, glm::vec3(0.0f, 0.0f, 1.0f));

  // Scale
  model = glm::scale(model, scale_);

  return model;
}

void Model3D::render() const {
  if (!visible_ || meshes_.empty()) {
    return;
  }

  for (const auto &mesh : meshes_) {
    if (mesh && mesh->isValid()) {
      mesh->draw();
    }
  }
}

} // namespace view
