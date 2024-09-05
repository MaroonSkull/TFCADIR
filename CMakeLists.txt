cmake_minimum_required (VERSION 3.30) #for find Boost

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TFCADIR"
    VERSION 0.2
    DESCRIPTION "the first CAD in Russia[, which has no alternatives]"
    HOMEPAGE_URL "https://github.com/MaroonSkull/TFCADIR"
    LANGUAGES CXX C
)

include (CMake/utils.cmake)

#embed-resource
add_subdirectory_checked (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/embed-resource)

# Create target
add_executable (${PROJECT_NAME})
target_compile_features (${PROJECT_NAME} PUBLIC cxx_std_17)

# include system libraries
find_package (OpenGL REQUIRED)
find_package (Boost #[[COMPONENTS statechart]] REQUIRED)
find_package (spdlog) # TODO: сделать проверку на наличие spdlog

add_subdirectory ("Dependencies")

add_subdirectory ("Src")

# Convert GLSL shaders to binary
embed_resources(MyResources # todo rename 1D to 2D
    Resources/glsl/1D/frag.glsl
    Resources/glsl/1D/vert.glsl
    Resources/glsl/1D/white_vert.glsl
)

target_sources (${PROJECT_NAME}
    PRIVATE
        Src/main.cpp

    PUBLIC
        ${MyResources}
)

# link libraries
foreach (library
# dependencies
    Boost::headers
    OpenGL::GL
    spdlog::spdlog
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