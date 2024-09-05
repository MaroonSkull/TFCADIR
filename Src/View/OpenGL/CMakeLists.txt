cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})

add_subdirectory ("Details")
add_subdirectory ("ImGUI")

add_library (ImGUIOpenGLView)
add_dependencies (${PROJECT_NAME} ImGUIOpenGLView)

target_sources (ImGUIOpenGLView
	PRIVATE
		ImGUI/ImGUI.cpp
	PUBLIC
		FILE_SET imGUIOpenGLViewHeaders
		TYPE HEADERS
		BASE_DIRS ../../
		FILES
			ImGUI.hpp
)

target_link_libraries(ImGUIOpenGLView
	PUBLIC
		Shader
		GLProgram
		GUI
		GLFWPP
		IView

	PRIVATE
		glad
		glm
		imgui
		EmbedResource
		spdlog::spdlog
		model
		controller
)

target_compile_features (ImGUIOpenGLView
	PUBLIC
		cxx_std_17
)

# install command needs to reference the file set again
install(TARGETS ImGUIOpenGLView FILE_SET imGUIOpenGLViewHeaders)