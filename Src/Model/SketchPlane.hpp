#pragma once

#include <glm/glm.hpp>
#include <string>

namespace model {

/**
 * @brief Defines a 2D sketching plane in 3D space
 * @details Represents a plane (XY, XZ, YZ, or custom) for 2D sketching
 * operations
 */
class SketchPlane {
public:
  /**
   * @brief Preset plane types for common sketching planes
   */
  enum class PresetPlane {
    XY, ///< Z = 0 plane (horizontal plane viewed from top)
    XZ, ///< Y = 0 plane (vertical plane viewed from front)
    YZ  ///< X = 0 plane (vertical plane viewed from side)
  };

private:
  glm::vec3 normal_{0.0f, 0.0f, 1.0f};
  glm::vec3 origin_{0.0f, 0.0f, 0.0f};
  glm::vec3 up_{0.0f, 1.0f, 0.0f};
  glm::vec3 right_{1.0f, 0.0f, 0.0f};
  std::string name_{"XY-Plane"};
  PresetPlane preset_{PresetPlane::XY};

public:
  /**
   * @brief Constructor for preset planes
   * @param preset The preset plane type to create
   */
  explicit SketchPlane(PresetPlane preset);

  /**
   * @brief Constructor for custom planes
   * @param normal The plane normal vector (should be normalized)
   * @param origin The origin point of the plane
   * @param name Human-readable name for the plane
   */
  SketchPlane(const glm::vec3 &normal, const glm::vec3 &origin,
              const std::string &name);

  // Getters

  /**
   * @brief Gets the plane normal vector
   * @return Const reference to the normal vector
   */
  const glm::vec3 &getNormal() const { return normal_; }

  /**
   * @brief Gets the plane origin point
   * @return Const reference to the origin vector
   */
  const glm::vec3 &getOrigin() const { return origin_; }

  /**
   * @brief Gets the plane up vector
   * @return Const reference to the up vector
   */
  const glm::vec3 &getUp() const { return up_; }

  /**
   * @brief Gets the plane right vector
   * @return Const reference to the right vector
   */
  const glm::vec3 &getRight() const { return right_; }

  /**
   * @brief Gets the human-readable name of the plane
   * @return Const reference to the plane name
   */
  const std::string &getName() const { return name_; }

  /**
   * @brief Gets the preset plane type
   * @return The preset plane type
   */
  PresetPlane getPreset() const { return preset_; }

  /**
   * @brief Calculates orthographic camera view matrix for this plane
   * @return 4x4 view matrix for orthographic viewing of the plane
   * @details Returns a lookAt matrix positioned to view the plane
   * perpendicularly
   */
  glm::mat4 getCameraViewMatrix() const;

  /**
   * @brief Gets the distance from a point to the plane
   * @param point The 3D point to measure distance from
   * @return Perpendicular distance from the point to the plane
   */
  float distanceToPoint(const glm::vec3 &point) const;

  /**
   * @brief Projects a 3D point onto the plane, returning 2D coordinates
   * @param point The 3D point to project
   * @return 2D coordinates on the plane (using right and up vectors as basis)
   */
  glm::vec2 projectPoint(const glm::vec3 &point) const;

  /**
   * @brief Unprojects 2D coordinates to a 3D point on the plane
   * @param coords 2D coordinates on the plane
   * @return 3D point on the plane
   */
  glm::vec3 unprojectPoint(const glm::vec2 &coords) const;

private:
  /**
   * @brief Calculates orthonormal basis vectors for the plane
   * @details Sets up right and up vectors perpendicular to the normal
   */
  void calculateBasisVectors();
};

} // namespace model
