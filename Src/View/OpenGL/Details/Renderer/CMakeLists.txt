add_library(Renderer
    AxesRenderer.cpp
    GridRenderer.cpp
)

target_include_directories(Renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Renderer
    GLProgram
    Shader
    glad
    glm::glm
)
