cmake_minimum_required(VERSION 3.8)
project(Logging)

# Create the Logging library
add_library(Logging STATIC
    ConsoleSink.cpp
    FileSink.cpp
    ImGUI.cpp
    LogViewer.cpp
    LoggerManager.cpp
)

# Set C++ standard
target_compile_features(Logging PUBLIC cxx_std_23)

# Include directories
target_include_directories(Logging
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link dependencies
target_link_libraries(Logging
    PUBLIC
        spdlog::spdlog
    PRIVATE
        imgui
        yaml-cpp
)

# Set compiler warnings
if(MSVC)
    target_compile_options(Logging PRIVATE /W4)
else()
    target_compile_options(Logging PRIVATE -Wall -Wextra -Wpedantic)
endif()
