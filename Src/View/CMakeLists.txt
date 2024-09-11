cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(IView
    INTERFACE
    PUBLIC_SOURCES
        IView.hpp
    INTERFACE_DEPENDENCIES
        Boost::headers
    COMPILE_FEATURES
        cxx_std_17
)

# выбор конкретного вью
add_subdirectory ("OpenGL")

add_library (View ALIAS ImGUIOpenGLView)