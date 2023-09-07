cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (controller STATIC openglImguiController.cpp)
add_dependencies (${PROJECT_NAME} controller)

target_include_directories (controller
	PRIVATE ./
	PUBLIC ${STAGING_DIR}/include/ # spdlog
)

target_link_libraries(controller
	PUBLIC
		GLFWPP
		imgui
		IController
	PRIVATE
		model
)

target_compile_features (controller PUBLIC cxx_std_17)