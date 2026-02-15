#pragma once

// MVC
#include <Controller/IController.hpp>
#include <Controller/OpenGL/ImGUI.hpp>
#include <IView.hpp>
#include <Model/FlatFigure.hpp>

// OpenGL
#include <glad/gl.h>
#include <glfwpp/glfwpp.h>
#include <glm/mat4x4.hpp>

// Details
#include "Details/RenderingPipeline3D.hpp"
#include <GLProgram.hpp>
#include <GUI.hpp>
#include <Shader.hpp>

class OpenglImguiView final : public IView {
  friend class OpenglImguiController; // wtf, i want to remove it. Looks like
                                      // dirty design
private:
  glfw::GlfwLibrary GLFW_; // RAII
  std::shared_ptr<model::FlatFigures> sp_model_;
  std::shared_ptr<controller::IController> sp_controller_;
  glfw::Window *Window_ = nullptr;
  GUI UI_;
  Shader *Vertex_ = nullptr;
  Shader *Fragment_ = nullptr;
  GLProgram *Pipeline_ = nullptr;
  GLsizei frameWidth_{};
  GLsizei frameHeight_{};
  // No EBO for now. If adding, could link to existing points.
  // Could also store Points in some std::set or std::[unordered_]map and store
  // indices in the model instead of points
  GLuint VAO_{};       // vertex array object
  GLuint VBO_{};       // vertex buffer object
  GLuint FBO_{};       // frame buffer object
  GLuint RBO_{};       // rendering buffer object
  GLuint textureId_{}; // the texture id we'll need later to create a texture
  // todo tmp, move to model
  std::vector<GLfloat> vertices_{};

  /// 3D rendering pipeline integrating all rendering components
  std::unique_ptr<view::RenderingPipeline3D> renderingPipeline_;

  // creates the vertex arrays and buffers
  void create_triangle();
  void create_framebuffer();
  void rescale_framebuffer();

  // TODO: make smthng like this
  // glm::mat4 transform(glm::vec2 const& Orientation, glm::vec3 const&
  // Translate, glm::vec3 const& Up);

public:
  // init glfwpp, glad, window, imgui
  OpenglImguiView(std::shared_ptr<model::FlatFigures>,
                  std::shared_ptr<controller::IController>);
  ~OpenglImguiView();

  void draw();
  inline bool shouldClose() const;
};
