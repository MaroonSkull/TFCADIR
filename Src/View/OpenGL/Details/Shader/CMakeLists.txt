cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})



config_lib(Shader
    PRIVATE_SOURCES
        Shader.cpp
    PUBLIC_SOURCES
        Shader.hpp
    PUBLIC_DEPENDENCIES
        glad
        EmbedResource
    COMPILE_FEATURES
        cxx_std_17
)
