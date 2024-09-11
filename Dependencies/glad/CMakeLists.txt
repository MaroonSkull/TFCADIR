# 3.23+ for config_lib
cmake_minimum_required (VERSION 3.23)
project (glad LANGUAGES C)

config_lib(glad
	BASE_DIR
		include
    PUBLIC_SOURCES
		include/glad/gl.h
		include/KHR/khrplatform.h
	PRIVATE_SOURCES
		src/gl.c
)