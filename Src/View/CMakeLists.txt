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

# Phase 4: Command System (Undo/Redo, Command History)
config_lib(Commands
    PRIVATE_SOURCES
        Commands/CommandFactory.cpp
        Commands/ExtendedCommandManager.cpp
        Commands/ColorChangeCommand.cpp
        Commands/CreateFigureCommand.cpp
        Commands/DeleteFiguresCommand.cpp
        Commands/DuplicateFigureCommand.cpp
        Commands/GroupFiguresCommand.cpp
        Commands/MacroCommand.cpp
        Commands/MoveFigureCommand.cpp
        Commands/PropertyChangeCommand.cpp
        Commands/RotateFigureCommand.cpp
        Commands/ScaleFigureCommand.cpp
        Commands/UngroupFiguresCommand.cpp
        Commands/ImGUI/CommandHistoryPanel.cpp
    PUBLIC_SOURCES
        Commands/ICommand.hpp
        Commands/CommandFactory.hpp
        Commands/ExtendedCommandManager.hpp
        Commands/ColorChangeCommand.hpp
        Commands/CreateFigureCommand.hpp
        Commands/DeleteFiguresCommand.hpp
        Commands/DuplicateFigureCommand.hpp
        Commands/GroupFiguresCommand.hpp
        Commands/MacroCommand.hpp
        Commands/MoveFigureCommand.hpp
        Commands/PropertyChangeCommand.hpp
        Commands/RotateFigureCommand.hpp
        Commands/ScaleFigureCommand.hpp
        Commands/UngroupFiguresCommand.hpp
        Commands/ImGUI/CommandHistoryPanel.hpp
    PUBLIC_DEPENDENCIES
        Model
        UIFSMAdapter
        imgui
    PRIVATE_DEPENDENCIES
        nlohmann_json::nlohmann_json
        spdlog::spdlog
    COMPILE_FEATURES
        cxx_std_17
)

# Select specific view
add_subdirectory ("OpenGL")

add_library (View ALIAS ImGUIOpenGLView)