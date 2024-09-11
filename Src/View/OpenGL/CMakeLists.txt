cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})

add_subdirectory ("Details")
add_subdirectory ("ImGUI")


config_lib(ImGUIOpenGLView
    BASE_DIR
        ../../
    PRIVATE_SOURCES
        ImGUI/ImGUI.cpp
    PUBLIC_SOURCES
        ImGUI.hpp
    PUBLIC_DEPENDENCIES
		Model
		ImGUIOpenGLController
		IController
		IView
		GUI
		Shader
		GLProgram
		GLFWPP
		glm
    PRIVATE_DEPENDENCIES
		spdlog::spdlog
		EmbedResource
		glad
		imgui
    COMPILE_FEATURES
        cxx_std_17
)