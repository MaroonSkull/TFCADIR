cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library (GUI STATIC GUI.cpp)
add_dependencies (${PROJECT_NAME} GUI)

target_sources (GUI
	PRIVATE
		GUI.cpp

	PUBLIC
		FILE_SET guiHeaders
		TYPE HEADERS
		FILES
			GUI.hpp
)

target_link_libraries(GUI
	PRIVATE
		glad
		GLFWPP
		imgui
)

target_compile_features (GUI PUBLIC cxx_std_17)

# install command needs to reference the file set again
install(TARGETS GUI FILE_SET guiHeaders)