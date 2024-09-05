cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library(IView INTERFACE)
add_dependencies (${PROJECT_NAME} IView)

target_sources (IView
    INTERFACE
        FILE_SET iViewHeaders
        TYPE HEADERS
        FILES
            IView.hpp
)

target_compile_features(IView
    INTERFACE
        cxx_std_17
)

# install command needs to reference the file set again
install(TARGETS IView FILE_SET iViewHeaders)

# выбор конкретного вью
add_subdirectory ("OpenGL")

add_library (view ALIAS ImGUIOpenGLView)