cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (Shader)
add_dependencies (${PROJECT_NAME} Shader)

target_sources (Shader
	PRIVATE
		Shader.cpp

	PUBLIC
		FILE_SET shaderHeaders
		TYPE HEADERS
		FILES
			Shader.hpp
)

target_link_libraries (Shader
	PRIVATE
		glad
		EmbedResource
)

target_compile_features (Shader PUBLIC cxx_std_17)

# install command needs to reference the file set again
install(TARGETS Shader FILE_SET shaderHeaders)