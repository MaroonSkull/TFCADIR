cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_subdirectory ("Shader")
add_subdirectory ("GLProgram")
add_subdirectory ("Camera")
add_subdirectory ("Renderer")

# RenderingPipeline3D library
add_library(RenderingPipeline3D
    RenderingPipeline3D.cpp
)

target_include_directories(RenderingPipeline3D PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderingPipeline3D
    Renderer
    Camera
    glad
    glm::glm
)