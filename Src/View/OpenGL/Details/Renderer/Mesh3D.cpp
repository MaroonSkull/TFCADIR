#include "Mesh3D.hpp"

#include <utility>

namespace view {

Mesh3D::Mesh3D() : vao_(0), vbo_(0), ebo_(0), indexCount_(0), vertexCount_(0) {
  initialize();
}

Mesh3D::Mesh3D(const std::vector<Vertex3D> &vertices,
               const std::vector<unsigned int> &indices)
    : vao_(0), vbo_(0), ebo_(0), indexCount_(0), vertexCount_(0) {
  initialize();
  loadData(vertices, indices);
}

Mesh3D::~Mesh3D() { cleanup(); }

Mesh3D::Mesh3D(Mesh3D &&other) noexcept
    : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_),
      indexCount_(other.indexCount_), vertexCount_(other.vertexCount_) {
  other.vao_ = 0;
  other.vbo_ = 0;
  other.ebo_ = 0;
  other.indexCount_ = 0;
  other.vertexCount_ = 0;
}

Mesh3D &Mesh3D::operator=(Mesh3D &&other) noexcept {
  if (this != &other) {
    cleanup();
    vao_ = other.vao_;
    vbo_ = other.vbo_;
    ebo_ = other.ebo_;
    indexCount_ = other.indexCount_;
    vertexCount_ = other.vertexCount_;
    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.indexCount_ = 0;
    other.vertexCount_ = 0;
  }
  return *this;
}

void Mesh3D::loadData(const std::vector<Vertex3D> &vertices,
                      const std::vector<unsigned int> &indices) {
  vertexCount_ = static_cast<GLsizei>(vertices.size());
  indexCount_ = static_cast<GLsizei>(indices.size());

  glBindVertexArray(vao_);

  // Upload vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D),
               vertices.data(), GL_STATIC_DRAW);

  // Upload index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Mesh3D::bind() const { glBindVertexArray(vao_); }

void Mesh3D::unbind() const { glBindVertexArray(0); }

void Mesh3D::draw() const {
  if (isValid()) {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }
}

void Mesh3D::initialize() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

  setupVertexAttributes();

  glBindVertexArray(0);
}

void Mesh3D::cleanup() {
  if (ebo_ != 0) {
    glDeleteBuffers(1, &ebo_);
    ebo_ = 0;
  }
  if (vbo_ != 0) {
    glDeleteBuffers(1, &vbo_);
    vbo_ = 0;
  }
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
  }
  indexCount_ = 0;
  vertexCount_ = 0;
}

void Mesh3D::setupVertexAttributes() {
  // Position attribute (location = 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        reinterpret_cast<void *>(offsetof(Vertex3D, position)));

  // Normal attribute (location = 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        reinterpret_cast<void *>(offsetof(Vertex3D, normal)));

  // Texture coordinate attribute (location = 2)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        reinterpret_cast<void *>(offsetof(Vertex3D, texCoord)));

  // Tangent attribute (location = 3)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        reinterpret_cast<void *>(offsetof(Vertex3D, tangent)));
}

} // namespace view
