cmake_minimum_required (VERSION 3.30) #for find Boost

#Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project ("TFCADIR"
    VERSION 0.2.1
    DESCRIPTION "the first CAD in Russia[, which has no alternatives]"
    HOMEPAGE_URL "https://github.com/MaroonSkull/TFCADIR"
    LANGUAGES CXX C
)

# Enable Hot Reload for MSVC compilers if supported (set after project() so compiler is determined)
if (POLICY CMP0141)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<CXX_COMPILER_ID:MSVC>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Add /utf-8 compiler flag for MSVC to fix Unicode encoding issues (required by fmt library)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include (CMake/utils.cmake)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

#embed - resource
add_subdirectory_checked (
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/embed-resource
)

#Create target
add_executable (${PROJECT_NAME})
target_compile_features (${PROJECT_NAME} PUBLIC cxx_std_23)

#include system libraries
find_package (OpenGL REQUIRED)
find_package (Boost) # Optional - not currently used, will be needed in future
find_package (spdlog) # TODO: add check for spdlog availability

add_subdirectory ("Dependencies")

add_subdirectory ("Src")

#Convert GLSL shaders to binary
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

#link libraries
foreach (library
#dependencies
    OpenGL::GL
    swogl::swogl
    spdlog::spdlog
    glad
    GLFWPP
    glm
    imgui
    EmbedResource
    fsmconfig
    nlohmann_json::nlohmann_json
#sources
    Logging
    Model
    Controller
    View
)

    target_link_libraries (${PROJECT_NAME}
        PRIVATE
            ${library}
            Tools
    )

endforeach (library)

# Link Boost if found (optional - not currently used)
if(TARGET Boost::headers)
    target_link_libraries(${PROJECT_NAME} PRIVATE Boost::headers)
endif()