/**
 * @file Model.hpp
 * @author Maroon Skull
 * @brief file contains model class/
 * @version 0.1
 * @date 2024-12-25
 *
 * @todo rename filename to GodModel.hpp?
 */

#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <spdlog/spdlog.h>

#include <string>
#include <variant>
#include <vector>

namespace model {

namespace CoordinateSystem {


struct Local_t {};
struct World_t {};
struct View_t {};
struct Clip_t {};
struct Screen_t {};

using type = std::variant<
  Local_t,
  World_t,
  View_t,
  Clip_t,
  Screen_t
>;

/**
 * @brief Координатная система
 * @details Класс представляет из себя хранилище координат,
 * позволяющий преобразовывать координаты между
 * - local space (or object space)
 * - world space
 * - view space (or eye space)
 * - clip space
 * - screen space
 * @todo viewport transform to screen space
 * @todo maybe move class to its own file and namespace utils or something?
 */
class Object {
  private:
    glm::mat4x4 modelMatrix{};
    glm::mat4x4 viewMatrix{};
    glm::mat4x4 projectionMatrix{};
    // TODO: viewport transform variable here

    type currentSystem{};
  public:
    Object(const Object &) = default;
    Object(Object &&) = default;
    Object &operator=(const Object &) = default;
    Object &operator=(Object &&) = default;

    Object() = default;
    Object(
      glm::mat4x4 modelMatrix,
      glm::mat4x4 viewMatrix,
      glm::mat4x4 projectionMatrix,
      type currentSystem = type::local
    )
    : modelMatrix(modelMatrix)
    , viewMatrix(viewMatrix)
    , projectionMatrix(projectionMatrix)
    , currentSystem(currentSystem)
    {}


    glm::mat4x4 getModelMatrix() const;
    glm::mat4x4 getViewMatrix() const;
    glm::mat4x4 getProjectionMatrix() const;

    void setModelMatrix(const glm::mat4x4& matrix);
    void setViewMatrix(const glm::mat4x4& matrix);
    void setProjectionMatrix(const glm::mat4x4& matrix);

  // Я хочу сделать полный двудольный граф, для этого хорошо подойдёт std::variant + std::visit
};

std::variant<type>

namespace literals {
  // i want to write user-defined literals for Object class, like _ls, _ws, _vs, _cs for coordinates.  
}

}

/**
 * @brief Model.
 * @details God class antipattern, but it works.
 */
class godModel {

};

}