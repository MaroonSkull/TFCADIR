cmake_minimum_required(VERSION 3.23)
project(${PROJECT_NAME})

config_lib(GUI
    PRIVATE_SOURCES
        GUI.cpp
        # Phase 2 Tools System - Core components
        ../../Tools/ToolManager.cpp
        ../../Tools/PreviewRenderer.cpp
        # Phase 2 Tools System - ImGUI components
        ../../Tools/ImGUI/CommandManager.cpp
        ../../Tools/ImGUI/ToolOptionsPanel.cpp
        ../../Tools/ImGUI/KeyboardShortcutManager.cpp
    PUBLIC_SOURCES
        GUI.hpp
    PUBLIC_DEPENDENCIES
        GLFWPP
        imgui
        ImGUIOpenGLController
        IController
        UIFSMAdapter
        CameraController
    COMPILE_FEATURES
        cxx_std_17
)
