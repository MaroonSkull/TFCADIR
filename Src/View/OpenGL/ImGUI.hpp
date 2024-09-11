#pragma once

// MVC
#include <IView.hpp>
#include <Controller/IController.hpp>
#include <Controller/OpenGL/ImGUI.hpp>
#include <Model/FlatFigure.hpp>

// OpenGL
#include <glad/gl.h>
#include <glfwpp/glfwpp.h>
#include <glm/mat4x4.hpp>

// Details
#include <GUI.hpp>
#include <Shader.hpp>
#include <GLProgram.hpp>



class OpenglImguiView final : public IView {
	friend class OpenglImguiController;
private:
	glfw::GlfwLibrary GLFW_; // RAII
	std::shared_ptr<model::FlatFigures> sp_model_;
	std::shared_ptr<IController> sp_controller_;
	glfw::Window* Window_ = nullptr;
	GUI UI_{};
	Shader* Vertex_ = nullptr;
	Shader* Fragment_ = nullptr;
	GLProgram* Pipeline_ = nullptr;
	GLsizei frameWidth_{};
	GLsizei frameHeight_{};
	// Пока что без EBO. Если добавлять, то можно сделать привязку к уже существующим точкам организовать.
	// Ещё можно Point's хранить в каком-нибунадо как-то дь std::set или std::[unordered_]map и хранить в модели не точки, а их индексы
	GLuint VAO_{}; // vertex array object
	GLuint VBO_{}; // vertex buffer object
	GLuint FBO_{}; // frame buffer object
	GLuint RBO_{}; // rendering buffer object
	GLuint textureId_{}; // the texture id we'll need later to create a texture
	// todo tmp, move to model
	std::vector<GLfloat> vertices_{};

	//creates the vertex arrays and buffers
	void create_triangle();
	void create_framebuffer();
	void rescale_framebuffer();
	glm::mat4 transform(glm::vec2 const& Orientation, glm::vec3 const& Translate, glm::vec3 const& Up);
public:
	// init glfwpp, glad, window, imgui
	OpenglImguiView(std::shared_ptr<model::FlatFigures>, std::shared_ptr<IController>);
	~OpenglImguiView();

	void draw();
	inline bool shouldClose() const;
};