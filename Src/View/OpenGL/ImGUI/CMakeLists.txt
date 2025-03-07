cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(GUI
	PRIVATE_SOURCES
		GUI.cpp
	PUBLIC_SOURCES
		GUI.hpp
	PUBLIC_DEPENDENCIES
		GLFWPP
		imgui
		ImGUIOpenGLController
		IController
	COMPILE_FEATURES
		cxx_std_17
)