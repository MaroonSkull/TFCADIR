# here we add all dependencies
cmake_minimum_required (VERSION 3.19)
project (${PROJECT_NAME} LANGUAGES CXX)

#glad
add_subdirectory (glad)
add_dependencies (${PROJECT_NAME} glad)

# GLFWPP
# glfwpp has public linkage of glfw
set (GLFWPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # disable building examples
add_subdirectory_checked (${CMAKE_CURRENT_SOURCE_DIR}/glfwpp)
add_dependencies (${PROJECT_NAME} GLFWPP)

# glm
add_subdirectory_checked (${CMAKE_CURRENT_SOURCE_DIR}/glm)
add_dependencies (${PROJECT_NAME} glm)

# imgui
add_library (imgui
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories (imgui PUBLIC
    imgui
    imgui/backends
)
target_link_libraries (imgui PUBLIC glfw glad)

if (WIN32 AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
    target_link_libraries(imgui "Imm32.lib")
endif()

add_dependencies (${PROJECT_NAME} imgui)

#embed-resource
add_library (EmbedResource INTERFACE)
target_include_directories (EmbedResource INTERFACE
    embed-resource
)
add_dependencies (${PROJECT_NAME} EmbedResource)