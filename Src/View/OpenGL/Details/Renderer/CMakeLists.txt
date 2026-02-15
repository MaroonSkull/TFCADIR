add_library(Renderer
    AxesRenderer.cpp
    GridRenderer.cpp
    Mesh3D.cpp
    Model3D.cpp
    ObjectRenderer.cpp
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
