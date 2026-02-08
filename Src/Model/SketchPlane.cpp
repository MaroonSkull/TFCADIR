#include "SketchPlane.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace model {

SketchPlane::SketchPlane(PresetPlane preset) : preset_(preset) {
  switch (preset) {
    case PresetPlane::XY:
      // Z = 0 plane (horizontal)
      normal_ = glm::vec3(0.0f, 0.0f, 1.0f);
      origin_ = glm::vec3(0.0f, 0.0f, 0.0f);
      name_ = "XY-Plane";
      break;
    case PresetPlane::XZ:
      // Y = 0 plane (vertical, front view)
      normal_ = glm::vec3(0.0f, 1.0f, 0.0f);
      origin_ = glm::vec3(0.0f, 0.0f, 0.0f);
      name_ = "XZ-Plane";
      break;
    case PresetPlane::YZ:
      // X = 0 plane (vertical, side view)
      normal_ = glm::vec3(1.0f, 0.0f, 0.0f);
      origin_ = glm::vec3(0.0f, 0.0f, 0.0f);
      name_ = "YZ-Plane";
      break;
  }
  calculateBasisVectors();
}

SketchPlane::SketchPlane(const glm::vec3& normal, const glm::vec3& origin,
                         const std::string& name)
    : normal_(glm::normalize(normal)), origin_(origin), name_(name),
      preset_(PresetPlane::XY) {
  calculateBasisVectors();
}

void SketchPlane::calculateBasisVectors() {
  // Calculate orthonormal basis for the plane
  // Start with an arbitrary vector that's not parallel to normal
  glm::vec3 arbitrary(0.0f, 1.0f, 0.0f);
  
  // If normal is parallel to arbitrary, choose different vector
  if (std::abs(glm::dot(normal_, arbitrary)) > 0.99f) {
    arbitrary = glm::vec3(1.0f, 0.0f, 0.0f);
  }
  
  // Calculate right vector (perpendicular to normal)
  right_ = glm::normalize(glm::cross(normal_, arbitrary));
  
  // Calculate up vector (perpendicular to both normal and right)
  up_ = glm::normalize(glm::cross(right_, normal_));
}

glm::mat4 SketchPlane::getCameraViewMatrix() const {
  // Position camera at a distance from the plane along the normal
  const float cameraDistance = 10.0f;
  glm::vec3 cameraPosition = origin_ + normal_ * cameraDistance;
  
  // Camera looks at the plane origin, with up as the plane's up vector
  return glm::lookAt(cameraPosition, origin_, up_);
}

float SketchPlane::distanceToPoint(const glm::vec3& point) const {
  // Distance from point to plane = dot(point - origin, normal)
  return glm::dot(point - origin_, normal_);
}

glm::vec2 SketchPlane::projectPoint(const glm::vec3& point) const {
  // Project 3D point onto plane, then express in 2D plane coordinates
  glm::vec3 pointOnPlane = point - normal_ * distanceToPoint(point);
  glm::vec3 offset = pointOnPlane - origin_;
  
  // Express offset in terms of right and up basis vectors
  float u = glm::dot(offset, right_);
  float v = glm::dot(offset, up_);
  
  return glm::vec2(u, v);
}

glm::vec3 SketchPlane::unprojectPoint(const glm::vec2& coords) const {
  // Convert 2D plane coordinates to 3D point on the plane
  return origin_ + right_ * coords.x + up_ * coords.y;
}

} // namespace model
