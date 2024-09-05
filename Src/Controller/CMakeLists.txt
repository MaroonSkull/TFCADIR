cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library (IController INTERFACE)
add_dependencies (${PROJECT_NAME} IController)

target_sources (IController
    INTERFACE
        FILE_SET icontrollerHeaders
        TYPE HEADERS
        FILES
            IController.hpp
)

target_compile_features (IController
    INTERFACE
        cxx_std_17
)

# install command needs to reference the file set again
install(TARGETS IController FILE_SET icontrollerHeaders)

# выбор конкретного контроллера
add_subdirectory ("OpenGL")

add_library (controller ALIAS ImGUIOpenGLController)