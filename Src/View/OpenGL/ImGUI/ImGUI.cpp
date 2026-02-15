#include "IController.hpp"
#include <View/OpenGL/ImGUI.hpp>

// #include <boost/mp11/bind.hpp>
// #include <boost/signals2.hpp>

#include <GLFW/glfw3.h>
#include <Resource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <spdlog/spdlog.h>

#include <sstream>

OpenglImguiView::OpenglImguiView(
    std::shared_ptr<model::FlatFigures> sp_model,
    std::shared_ptr<controller::IController> sp_controller)
    : GLFW_{glfw::init()}, UI_{sp_controller_}, sp_model_{sp_model},
      sp_controller_{sp_controller} {
  if (sp_model_ == nullptr || sp_controller_ == nullptr)
    throw std::invalid_argument{
        "sp_model or sp_controller can't be nullptr in view constructor!"};
  try {

    // mustBeRedrawSignal.connect(boost::bind(&OpenglImguiView::draw, this));

    // init glfwpp, glad, window, imgui
    glfw::InitHints iHints;
    // iHints.platform = glfw::GlfwPlatform::Wayland; // Let GLFW auto-detect
    // platform
    iHints.apply();

    // Window
    glfw::WindowHints wHints;
    wHints.clientApi = glfw::ClientApi::OpenGl;
    wHints.openglProfile = glfw::OpenGlProfile::Core;
    wHints.contextVersionMajor = 3;
    wHints.contextVersionMinor = 3;
    wHints.openglForwardCompat = true;
    wHints.apply();

    Window_ = new glfw::Window{640, 480, "Interview test"};
    // first framebuffer pass renders at window size
    std::tie(frameWidth_, frameHeight_) = Window_->getFramebufferSize();
    glfw::makeContextCurrent(*Window_);
    glfw::swapInterval(1); // Enable vsync

    int version = gladLoadGL(glfw::getProcAddress);
    if (version == 0)
      throw std::runtime_error{"Failed to initialize OpenGL context"};

    spdlog::info("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version),
                 GLAD_VERSION_MINOR(version));

    glViewport(0, 0, frameWidth_, frameHeight_);
    create_triangle();

    // Initialize 3D rendering pipeline
    renderingPipeline_ = std::make_unique<view::RenderingPipeline3D>();
    if (!renderingPipeline_->initialize(frameWidth_, frameHeight_)) {
      throw std::runtime_error{"Failed to initialize 3D rendering pipeline"};
    }
    spdlog::info("3D rendering pipeline initialized successfully");

    // Shaiders
    Fragment_ = new Shader(LOAD_RESOURCE(Resources_glsl_1D_frag_glsl),
                           Shader::Fragment);
    Vertex_ =
        new Shader(LOAD_RESOURCE(Resources_glsl_1D_vert_glsl), Shader::Vertex);
    // Vertex_ = new Shader(LOAD_RESOURCE(Resources_glsl_1D_white_vert_glsl),
    // Shader::Vertex);

    Pipeline_ = new GLProgram();
    Pipeline_->attachShader(Fragment_);
    Pipeline_->attachShader(Vertex_);
    Pipeline_->linkProgram();
    Fragment_->deleteShader();
    Vertex_->deleteShader();

    // todo looks like there's a bug here
    // need to generate two framebuffers and render to them in turns
    create_framebuffer();

    // Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; //
    // https://github.com/ocornut/imgui/issues/6281

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(*Window_, true))
      throw std::runtime_error{"ImGui_ImplGlfw_InitForOpenGL return false."};
    if (!ImGui_ImplOpenGL3_Init("#version 330"))
      throw std::runtime_error{"ImGui_ImplOpenGL3_Init return false."};

  } catch (const glfw::Error &e) {
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
  vertices_ = {{
      -1.0f, -1.0f, 0.0f, // 1. vertex x, y, z
      1.0f, -1.0f, 0.0f,  // 2. vertex ...
      0.0f, 1.0f, 0.0f    // etc...
  }};

  glGenVertexArrays(1, &VAO_);
  glBindVertexArray(VAO_);

  glGenBuffers(1, &VBO_);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_);
  // GL_DYNAMIC_DRAW - we will be changing data intensively
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_.at(0)),
               vertices_.data(), GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void OpenglImguiView::create_framebuffer() {
  glGenFramebuffers(1, &FBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO_);

  // Texture to which we will render
  glGenTextures(1, &textureId_);
  // Renderbuffer for OpenGL purposes
  glGenRenderbuffers(1, &RBO_);
  rescale_framebuffer();

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::stringstream err;
    err << "framebuffer is not complete, glCheckFramebufferStatus = "
        << glCheckFramebufferStatus(GL_FRAMEBUFFER);
    throw std::runtime_error{err.str()};
  }

  // unbind it all
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void OpenglImguiView::rescale_framebuffer() {
  glBindTexture(GL_TEXTURE_2D, textureId_);
  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth_, frameHeight_, 0, GL_RGB,
               GL_UNSIGNED_BYTE, 0);
  // Poor filtering. Needed !
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureId_, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, RBO_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frameWidth_,
                        frameHeight_);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, RBO_);
}

void OpenglImguiView::draw() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  // reinterpret_cast is inevitable here, it's a necessary measure to pass
  // the texture from GLFWPP to imgui
  auto [frameSizes, momentWheel, mousePosition] =
      UI_.DrawGUI(reinterpret_cast<ImTextureID>(textureId_));

  // todo observer would be useful here. If model and view haven't changed,
  // don't render new texture

  /// Skip OpenGL rendering when canvas is hidden/collapsed (zero dimensions)
  /// This prevents GL_INVALID_FRAMEBUFFER_OPERATION errors (GLAD error 1286)
  /// that occur when attempting to render to a framebuffer with invalid
  /// dimensions
  const bool canvasVisible = (frameSizes.x > 0 && frameSizes.y > 0);

  if (canvasVisible) {
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

    // handling direct input operations via canvas
    // maybe wrap in lambda, create callback
    if (mousePosition.has_value()) {
      const auto &[x, y] = mousePosition.value();
      // to ndc and to model
      // sp_model_->camera_

      sp_controller_->updateWorkspaceHoverState(
          controller::state::Workspace::
              hovered /*, x / frameWidth_, y / frameHeight_ */);

      if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        sp_controller_->updateLeftMouseButtonState(
            controller::state::Button::down);
      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        sp_controller_->updateLeftMouseButtonState(
            controller::state::Button::released);

      if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        sp_controller_->updateRightMouseButtonState(
            controller::state::Button::down);
      if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        sp_controller_->updateRightMouseButtonState(
            controller::state::Button::released);

      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        sp_controller_->updateWheelMouseButtonState(
            controller::state::Button::down);
      if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
        sp_controller_->updateWheelMouseButtonState(
            controller::state::Button::released);

      if (momentWheel != 0.0f)
        sp_controller_->updateScroll(momentWheel);

      // convert to normalised coords via glm
    } else
      sp_controller_->updateWorkspaceHoverState(
          controller::state::Workspace::unhovered);

    // Process camera controls input before rendering
    if (renderingPipeline_ && renderingPipeline_->isInitialized()) {
      // Update viewport if needed
      renderingPipeline_->onViewportResize(frameWidth_, frameHeight_);

      /// Get ImGui IO to check for input capture
      /// This prevents camera movement when interacting with ImGui UI elements
      /// (sliders, input fields, etc.)
      ImGuiIO &io = ImGui::GetIO();
      const bool imguiWantsMouse = io.WantCaptureMouse;
      const bool imguiWantsKeyboard = io.WantCaptureKeyboard;

      /// Only process camera input when ImGui doesn't want to capture it
      /// This ensures camera doesn't move during UI interactions
      if (!imguiWantsMouse && mousePosition.has_value()) {
        const auto &[x, y] = mousePosition.value();
        auto &orbitControls = renderingPipeline_->getOrbitControls();

        // Handle mouse button state changes for camera orbit/pan
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          orbitControls.onMouseButton(0, 1,
                                      0); // GLFW_MOUSE_BUTTON_1, GLFW_PRESS
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
          orbitControls.onMouseButton(0, 0,
                                      0); // GLFW_MOUSE_BUTTON_1, GLFW_RELEASE
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
          orbitControls.onMouseButton(2, 1,
                                      0); // GLFW_MOUSE_BUTTON_3, GLFW_PRESS
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
          orbitControls.onMouseButton(2, 0,
                                      0); // GLFW_MOUSE_BUTTON_3, GLFW_RELEASE
        }

        // Handle mouse movement for orbit/pan
        orbitControls.onCursorPos(static_cast<double>(x),
                                  static_cast<double>(y));

        // Handle scroll for zoom
        if (momentWheel != 0.0f) {
          orbitControls.onScroll(static_cast<double>(momentWheel));
        }
      }

      /// Process keyboard input for camera controls when ImGui doesn't want
      /// keyboard This allows keyboard camera controls (Arrow keys, W/S/A/D, R)
      /// to work only when not typing in ImGui input fields
      if (!imguiWantsKeyboard) {
        auto &orbitControls = renderingPipeline_->getOrbitControls();

        /// Forward keyboard input to OrbitControls
        /// Keys: Arrow keys (orbit), W/S (zoom), A/D (pan), R (reset view)
        /// Using ImGuiKey enum for cross-platform compatibility
        /// ImGui converts GLFW keys internally via
        /// ImGui_ImplGlfw_KeyToImGuiKey()
        struct KeyMapping {
          ImGuiKey imguiKey;
          int glfwKey;
        };
        const KeyMapping cameraKeys[] = {{ImGuiKey_LeftArrow, GLFW_KEY_LEFT},
                                         {ImGuiKey_RightArrow, GLFW_KEY_RIGHT},
                                         {ImGuiKey_UpArrow, GLFW_KEY_UP},
                                         {ImGuiKey_DownArrow, GLFW_KEY_DOWN},
                                         {ImGuiKey_W, GLFW_KEY_W},
                                         {ImGuiKey_S, GLFW_KEY_S},
                                         {ImGuiKey_A, GLFW_KEY_A},
                                         {ImGuiKey_D, GLFW_KEY_D},
                                         {ImGuiKey_R, GLFW_KEY_R}};

        for (const auto &key : cameraKeys) {
          // Handle key press (single event)
          if (ImGui::IsKeyPressed(key.imguiKey)) {
            orbitControls.onKey(key.glfwKey, GLFW_PRESS, 0);
          }
          // Handle key repeat for continuous movement
          if (ImGui::IsKeyDown(key.imguiKey)) {
            orbitControls.onKey(key.glfwKey, GLFW_REPEAT, 0);
          }
        }
      }

      // Render the 3D world (grid, axes, objects)
      renderingPipeline_->render();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    glfw::Window &backupCurrentContext = glfw::getCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfw::makeContextCurrent(backupCurrentContext);
  }

  // swap buffer for next time
  Window_->swapBuffers();
  // update FPS display
  std::stringstream title;
  title << "Interview test | average FPS : "
        << static_cast<uint32_t>(ImGui::GetIO().Framerate)
        << ", momental FPS : "
        << static_cast<uint32_t>(1.0f / ImGui::GetIO().DeltaTime);
  Window_->setTitle(title.str().c_str());
}

inline bool OpenglImguiView::shouldClose() const {
  return Window_->shouldClose();
}