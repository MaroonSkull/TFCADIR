cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library(IController INTERFACE)
add_dependencies (${PROJECT_NAME} IController)

target_include_directories(IController INTERFACE ./)

target_compile_features(IController INTERFACE cxx_std_17)

# выбор конкретного контроллера
add_subdirectory ("OpenGLController")