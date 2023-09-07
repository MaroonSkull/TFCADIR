cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})

add_subdirectory ("GUI")

add_library (view STATIC openglImguiView.cpp)
add_dependencies (${PROJECT_NAME} view)

target_include_directories (view
	PRIVATE ./
	PUBLIC ${STAGING_DIR}/include/ # spdlog
)

target_link_libraries(view
	PUBLIC
		glad
		GLFWPP
		glm
		imgui
		EmbedResource
		IView
		Shader
		GLProgram
		GUI
	PRIVATE
		model
		controller
)

target_compile_features (view PUBLIC cxx_std_17)