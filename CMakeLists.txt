cmake_minimum_required (VERSION 3.13) #for target_sources relative patch

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TopSystems"
    VERSION 0.1
    DESCRIPTION "Решение тестового задания от ЗАО \"Топ Системы\" на позицию \"Разработчик С++\""
    HOMEPAGE_URL "https://github.com/MaroonSkull/TopSystems"
    LANGUAGES CXX
)

include (CMake/utils.cmake)

#embed-resource
add_subdirectory_checked (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/embed-resource)

# Create target
add_executable (${PROJECT_NAME})
target_compile_features (${PROJECT_NAME} PUBLIC cxx_std_17)

# include OpenGL
find_package (OpenGL REQUIRED)

add_subdirectory ("Dependencies")

target_include_directories (${PROJECT_NAME}
    PUBLIC TopSystemsTest
    PUBLIC glm
    PUBLIC ${STAGING_DIR}/include/ # spdlog
)

add_subdirectory ("TopSystemsTest")

# Convert GLSL shaders to binary
embed_resources(MyResources # todo rename 1D to 2D
    Resources/glsl/1D/frag.glsl
    Resources/glsl/1D/vert.glsl
    Resources/glsl/1D/white_vert.glsl
)

target_sources (${PROJECT_NAME} PUBLIC
    TopSystemsTest/TopSystemsTest.cpp
    ${MyResources}
)

# link libraries
foreach (library
# dependencies
    #OpenGL::GL
    glad
    GLFWPP
    glm
    imgui
    EmbedResource
# sources
    model
    controller
    view
)
    target_link_libraries (${PROJECT_NAME} PRIVATE ${library})
endforeach ()