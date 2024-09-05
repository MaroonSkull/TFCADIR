cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library (GLProgram)
add_dependencies (${PROJECT_NAME} GLProgram)

target_sources (GLProgram
	PRIVATE
		GLProgram.cpp
	
	PUBLIC
		FILE_SET glProgramHeaders
		TYPE HEADERS
		FILES
			GLProgram.hpp
)

target_link_libraries (GLProgram
	PUBLIC
		Shader
	PRIVATE
		glad
		EmbedResource
)

target_compile_features (GLProgram PUBLIC cxx_std_17)

# install command needs to reference the file set again
install(TARGETS GLProgram FILE_SET glProgramHeaders)