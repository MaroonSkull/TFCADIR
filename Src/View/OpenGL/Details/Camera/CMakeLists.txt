add_library(Camera
    Camera3D.cpp
    OrbitControls.cpp
)

target_include_directories(Camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Camera
    glm::glm
    glfw
)
