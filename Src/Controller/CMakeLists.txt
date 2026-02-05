cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(IController
    INTERFACE
    PUBLIC_SOURCES
        IController.hpp
    COMPILE_FEATURES
        cxx_std_17
)

# Select specific controller
add_subdirectory ("OpenGL")

add_library (Controller ALIAS ImGUIOpenGLController)