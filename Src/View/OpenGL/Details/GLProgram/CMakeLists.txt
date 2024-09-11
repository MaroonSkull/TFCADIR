cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(GLProgram
    PRIVATE_SOURCES
		GLProgram.cpp
    PUBLIC_SOURCES
		GLProgram.hpp
    PUBLIC_DEPENDENCIES
        Shader
    COMPILE_FEATURES
        cxx_std_17
)
