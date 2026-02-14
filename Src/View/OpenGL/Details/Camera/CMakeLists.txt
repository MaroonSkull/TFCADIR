add_library(Camera
    Camera3d.cpp
    OrbitControls.cpp
)

target_include_directories(Camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Camera
    glm::glm
)
