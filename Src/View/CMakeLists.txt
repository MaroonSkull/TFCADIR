cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(IView
    INTERFACE
    PUBLIC_SOURCES
        IView.hpp
    INTERFACE_DEPENDENCIES
        Boost::headers
    COMPILE_FEATURES
        cxx_std_17
)

# CameraController and UIFSMAdapter libraries for Phase 1
config_lib(CameraController
    PRIVATE_SOURCES
        CameraController.cpp
    PUBLIC_SOURCES
        CameraController.hpp
    PUBLIC_DEPENDENCIES
        Model
    COMPILE_FEATURES
        cxx_std_17
)

config_lib(UIFSMAdapter
    PRIVATE_SOURCES
        UIFSMAdapter.cpp
    PUBLIC_SOURCES
        UIFSMAdapter.hpp
    PUBLIC_DEPENDENCIES
        CameraController
        Controller
    COMPILE_FEATURES
        cxx_std_17
)

# Phase 3: Object Management UI components
config_lib(ObjectManagement
    PRIVATE_SOURCES
        ObjectManagement/SelectionManager.cpp
        ObjectManagement/CanvasHitTester.cpp
        ObjectManagement/ImGUI/OutlinerPanel.cpp
        ObjectManagement/ImGUI/PropertyInspectorPanel.cpp
    PUBLIC_SOURCES
        ObjectManagement/SelectionManager.hpp
        ObjectManagement/CanvasHitTester.hpp
        ObjectManagement/ImGUI/OutlinerPanel.hpp
        ObjectManagement/ImGUI/PropertyInspectorPanel.hpp
    PUBLIC_DEPENDENCIES
        UIFSMAdapter
        Model
        imgui
    COMPILE_FEATURES
        cxx_std_17
)

# Select specific view
add_subdirectory ("OpenGL")

add_library (View ALIAS ImGUIOpenGLView)