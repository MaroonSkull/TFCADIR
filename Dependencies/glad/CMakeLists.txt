# glad config, use C language
cmake_minimum_required (VERSION 3.19)
project (glad LANGUAGES C)

# glad
add_library (glad
	include/glad/gl.h
	src/gl.c
)

target_include_directories (glad PUBLIC include)