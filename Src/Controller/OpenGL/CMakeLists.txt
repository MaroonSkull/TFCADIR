cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library (ImGUIOpenGLController)
add_dependencies (${PROJECT_NAME} ImGUIOpenGLController)

target_sources (ImGUIOpenGLController
	PRIVATE 
		ImGUI/ImGUI.cpp
	
	PUBLIC
		FILE_SET imGUIOpenGLControllerHeaders
		TYPE HEADERS
		BASE_DIRS ../../
		FILES
			ImGUI.hpp
)

target_link_libraries(ImGUIOpenGLController
	PRIVATE
		GLFWPP
		imgui
		spdlog::spdlog
		model

	PUBLIC
		IController
)

target_compile_features (ImGUIOpenGLController
	PUBLIC
		cxx_std_17
)

# install command needs to reference the file set again
install(TARGETS ImGUIOpenGLController FILE_SET imGUIOpenGLControllerHeaders)