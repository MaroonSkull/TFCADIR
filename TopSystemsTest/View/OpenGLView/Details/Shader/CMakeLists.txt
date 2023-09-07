cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (Shader STATIC Shader.cpp)
add_dependencies (${PROJECT_NAME} Shader)

target_include_directories (Shader PUBLIC ./)

target_link_libraries (Shader
	PRIVATE
		glad
		EmbedResource
)

target_compile_features (Shader PUBLIC cxx_std_17)