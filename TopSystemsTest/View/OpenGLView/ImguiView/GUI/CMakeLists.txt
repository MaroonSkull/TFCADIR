cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (GUI STATIC GUI.cpp)
add_dependencies (${PROJECT_NAME} GUI)

target_include_directories (GUI PUBLIC ./)

target_link_libraries(GUI
	PRIVATE
		glad
		GLFWPP
		imgui
#		model
#		controller
)

target_compile_features (GUI PUBLIC cxx_std_17)