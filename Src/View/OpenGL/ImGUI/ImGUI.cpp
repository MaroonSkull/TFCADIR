#include <View/OpenGL/ImGUI.hpp>

#include <boost/mp11/bind.hpp>
#include <boost/signals2.hpp>

#include <Resource.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <spdlog/spdlog.h>

#include <sstream>



OpenglImguiView::OpenglImguiView(
	std::shared_ptr<model::FlatFigures> sp_model,
	std::shared_ptr<IController> sp_controller
)
	: GLFW_{ glfw::init() }
	, sp_model_{ sp_model }
	, sp_controller_{ sp_controller }
{
	if (sp_model_ == nullptr || sp_controller_ == nullptr)
		throw std::invalid_argument {
			"sp_model or sp_controller can't be nullptr in view constructor!"
		};
	try {
		
		mustBeRedrawSignal.connect(boost::bind(&OpenglImguiView::draw, this));

		// init glfwpp, glad, window, imgui
		glfw::InitHints iHints;
		iHints.platform = glfw::GlfwPlatform::Wayland;
		iHints.apply();

		// Window
		glfw::WindowHints wHints;
		wHints.clientApi = glfw::ClientApi::OpenGl;
		wHints.openglProfile = glfw::OpenGlProfile::Core;
		wHints.contextVersionMajor = 3;
		wHints.contextVersionMinor = 3;
		wHints.openglForwardCompat = true;
		wHints.apply();

		Window_ = new glfw::Window{ 640, 480, "Interview test" };
		// первый проход фреймбуфера рендерим в размерах самого окна.
		std::tie(frameWidth_, frameHeight_) = Window_->getFramebufferSize();
		glfw::makeContextCurrent(*Window_);
		glfw::swapInterval(1); // Enable vsync

		int version = gladLoadGL(glfw::getProcAddress);
		if (version == 0)
			throw std::runtime_error{ "Failed to initialize OpenGL context" };

		spdlog::info("Loaded OpenGL {}.{}",
			GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));


		glViewport(0, 0, frameWidth_, frameHeight_);
		create_triangle();

		// Shaiders
		Fragment_ = new Shader(LOAD_RESOURCE(Resources_glsl_1D_frag_glsl), Shader::Fragment);
		Vertex_ = new Shader(LOAD_RESOURCE(Resources_glsl_1D_vert_glsl), Shader::Vertex);
		//Vertex_ = new Shader(LOAD_RESOURCE(Resources_glsl_1D_white_vert_glsl), Shader::Vertex);

		Pipeline_ = new GLProgram();
		Pipeline_->attachShader(Fragment_);
		Pipeline_->attachShader(Vertex_);
		Pipeline_->linkProgram();
		Fragment_->deleteShader();
		Vertex_->deleteShader();

		// todo Похоже тут есть баг
		// надо генерировать два фреймбуффера и в них по очереди отрисовывать
		create_framebuffer();

		// Imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // https://github.com/ocornut/imgui/issues/6281
		

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle &style = ImGui::GetStyle();
		if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		// Setup Platform/Renderer backends
		if (!ImGui_ImplGlfw_InitForOpenGL(*Window_, true))
			throw std::runtime_error{ "ImGui_ImplGlfw_InitForOpenGL return false." };
		if (!ImGui_ImplOpenGL3_Init("#version 330"))
			throw std::runtime_error{ "ImGui_ImplOpenGL3_Init return false." };

	}
	catch (const glfw::Error& e) {
		// todo replace "View" with file name macro
		spdlog::error("glfwpp exception in View!");
		throw e;
	}
}

OpenglImguiView::~OpenglImguiView() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteFramebuffers(1, &FBO_);
	glDeleteTextures(1, &textureId_);
	glDeleteRenderbuffers(1, &RBO_);
	// todo glDeleteVertexArrays(...

	delete Pipeline_;

	delete Vertex_;
	delete Fragment_;

	delete Window_;
	// GlfwLibrary destructor calls glfwTerminate automatically
}

void OpenglImguiView::create_triangle() {
	vertices_ = { {
		-1.0f, -1.0f, 0.0f, // 1. vertex x, y, z
		1.0f, -1.0f, 0.0f, // 2. vertex ...
		0.0f, 1.0f, 0.0f // etc... 
	} };

	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);

	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	// GL_DYNAMIC_DRAW - будем интенсивно менять данные.
	//glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_.at(0)), vertices_.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void OpenglImguiView::create_framebuffer() {
	glGenFramebuffers(1, &FBO_);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_);

	// Текстура, в которую мы будем рендерить
	glGenTextures(1, &textureId_);
	// Рендербуфер для целей OpenGL
	glGenRenderbuffers(1, &RBO_);
	rescale_framebuffer();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::stringstream err;
		err << "framebuffer is not complete, glCheckFramebufferStatus = " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
		throw std::runtime_error{ err.str() };
	}

	// unbind it all
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void OpenglImguiView::rescale_framebuffer() {
	glBindTexture(GL_TEXTURE_2D, textureId_);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth_, frameHeight_, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId_, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RBO_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frameWidth_, frameHeight_);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_);
}

void OpenglImguiView::draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	// reinterpret_cast тут неизбежен, это вынужденная мера, чтобы передать opengl текстуру из GLFWPP в imgui
	auto [frameSizes, momentWheel, mousePosition] = UI_.DrawGUI(reinterpret_cast<ImTextureID>(textureId_));
	

	// todo тут пригодится observer. Если модель и вью не изменились, новую текстуру не рендерим

	// Render on the whole framebuffer
	glViewport(0, 0, frameWidth_, frameHeight_);
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_);
	// Rescale binded framebuffer
	if (frameWidth_ != frameSizes.x || frameHeight_ != frameSizes.y) {
		frameWidth_ = frameSizes.x;
		frameHeight_ = frameSizes.y;
		rescale_framebuffer();
	}

	// обработка операций непосредственного ввода с помощью канваса
	// мб в лямбду завернуть, коллбэк оформить 
	if (mousePosition) {
		const auto &[x, y] = mousePosition.value();
		// to ndc and to model
		sp_model_->camera_
		sp_controller_->onMouseHover(IController::InputState::hovered, x / frameWidth_, y / frameHeight_);

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) sp_controller_->onLeftMouseButton(IController::InputState::down);
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) sp_controller_->onLeftMouseButton(IController::InputState::released);

		if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) sp_controller_->onRightMouseButton(IController::InputState::down);
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) sp_controller_->onRightMouseButton(IController::InputState::released);

		if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) sp_controller_->onWheelMouseButton(IController::InputState::down);
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) sp_controller_->onWheelMouseButton(IController::InputState::released);

		if (momentWheel != 0.0f) sp_controller_->onScroll(momentWheel);

		// convert to normalised coords via glm
	}
	else sp_controller_->onMouseHover(IController::InputState::unhovered, 0.0f, 0.0f);
	

	glClearColor(0.9f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Pipeline_->useProgram();

	// TODO: remove, temporary raw operations
	sp_model_->model_ = glm::rotate(sp_model_->model_, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	const auto view = glm::lookAt(
    sp_model_->camera_.position,
		sp_model_->camera_.target,
		sp_model_->camera_.up
	);
	sp_model_->projection_ = glm::perspective(glm::radians(45.0f), 1.33f, 0.1f, 100.0f);

	if (const auto modelLoc = Pipeline_->getUniformLocation("model"); modelLoc.has_value())
		glUniformMatrix4fv(modelLoc.value(), 1, GL_FALSE, glm::value_ptr(sp_model_->model_));
	if (const auto viewLoc = Pipeline_->getUniformLocation("view"); viewLoc.has_value())
		glUniformMatrix4fv(viewLoc.value(), 1, GL_FALSE, glm::value_ptr(view));
	if (const auto projectionLoc = Pipeline_->getUniformLocation("projection"); projectionLoc.has_value())
		glUniformMatrix4fv(projectionLoc.value(), 1, GL_FALSE, glm::value_ptr(sp_model_->projection_));

	glBindVertexArray(VAO_);
	
	if (mousePosition) {
		// получаем данные из модели и отправляем в опенгл
		glBindBuffer(GL_ARRAY_BUFFER, VBO_);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(vertices_.at(0)), vertices_.data());
		glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_.at(0)), vertices_.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}
	glDrawArrays(GL_TRIANGLES, 0, 9);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		glfw::Window& backupCurrentContext = glfw::getCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfw::makeContextCurrent(backupCurrentContext);
	}

	// меняем буфер для следующего раза
	Window_->swapBuffers();
	// обновляем показания FPS
	std::stringstream title;
	title << "Interview test | average FPS : " << static_cast<uint32_t>(ImGui::GetIO().Framerate)
		  <<               ", momental FPS : " << static_cast<uint32_t>(1.0f / ImGui::GetIO().DeltaTime);
	Window_->setTitle(title.str().c_str());
}

inline bool OpenglImguiView::shouldClose() const {
	return Window_->shouldClose();
}