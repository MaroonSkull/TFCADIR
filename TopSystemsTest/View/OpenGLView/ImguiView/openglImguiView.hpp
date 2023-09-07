#pragma once
#include <glad/gl.h>
#include <glfwpp/glfwpp.h>
#include <glm/mat4x4.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <Resource.h>

#include <GUI.hpp>

#include <IView.hpp>
#include <Shader.hpp>
#include <GLProgram.hpp>

#include <OpenGLController/ImguiController/openglImguiController.hpp>
#include <flatFigureModel.hpp>

class OpenglImguiView final : public IView {
	friend class OpenglImguiController;
private:
	glfw::GlfwLibrary GLFW_; // RAII
	model::FlatFigures* pModel_ = nullptr;
	IController* pController_ = nullptr;
	glfw::Window* Window_ = nullptr;
	GUI UI_{};
	Shader* Vertex_ = nullptr;
	Shader* Fragment_ = nullptr;
	GLProgram* Pipeline_ = nullptr;
	GLsizei frameWidth_{};
	GLsizei frameHeight_{};
	// Пока что без EBO. Если добавлять, то можно сделать привязку к уже существующим точкам организовать.
	// Ещё можно Point's хранить в каком-нибудь std::set или std::[unordered_]map и хранить в модели не точки, а их индексы
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
	OpenglImguiView(model::FlatFigures*, IController*);
	~OpenglImguiView();

	void draw();
	inline bool shouldClose() const;
};