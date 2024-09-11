cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(ImGUIOpenGLController
    BASE_DIR
        ../../
    PRIVATE_SOURCES
        ImGUI/ImGUI.cpp
    PUBLIC_SOURCES
        ImGUI.hpp
    PUBLIC_DEPENDENCIES
        Model
        IController
    PRIVATE_DEPENDENCIES
        Boost::headers
        ImGUIOpenGLView
        imgui
        GLFWPP
        spdlog::spdlog
    COMPILE_FEATURES
        cxx_std_17
)