cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (GLProgram STATIC GLProgram.cpp)
add_dependencies (${PROJECT_NAME} GLProgram)

target_include_directories (GLProgram PUBLIC ./)

target_link_libraries (GLProgram
	PUBLIC
		Shader
	PRIVATE
		glad
		EmbedResource
)

target_compile_features (GLProgram PUBLIC cxx_std_17)