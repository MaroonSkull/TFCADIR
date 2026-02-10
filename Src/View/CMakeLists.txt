cmake_minimum_required(VERSION 3.23)
project(${PROJECT_NAME})

    config_lib(IView INTERFACE PUBLIC_SOURCES IView.hpp INTERFACE_DEPENDENCIES
                   Boost::headers COMPILE_FEATURES cxx_std_17)

#CameraController and UIFSMAdapter libraries for Phase 1
        config_lib(CameraController PRIVATE_SOURCES
                       CameraController.cpp PUBLIC_SOURCES
                           CameraController.hpp PUBLIC_DEPENDENCIES Model
                               COMPILE_FEATURES cxx_std_17)

            config_lib(UIFSMAdapter PRIVATE_SOURCES UIFSMAdapter
                           .cpp PUBLIC_SOURCES UIFSMAdapter
                           .hpp PUBLIC_DEPENDENCIES CameraController Controller
                               tinyexpr COMPILE_FEATURES cxx_std_17)

#Phase 3 : Object Management UI components
                config_lib(
                    ObjectManagement PRIVATE_SOURCES ObjectManagement /
                    SelectionManager.cpp ObjectManagement /
                    CanvasHitTester.cpp ObjectManagement / ImGUI /
                    OutlinerPanel.cpp ObjectManagement / ImGUI /
                    PropertyInspectorPanel.cpp PUBLIC_SOURCES ObjectManagement /
                    SelectionManager.hpp ObjectManagement /
                    CanvasHitTester.hpp ObjectManagement / ImGUI /
                    OutlinerPanel.hpp ObjectManagement / ImGUI /
                    PropertyInspectorPanel.hpp PUBLIC_DEPENDENCIES UIFSMAdapter
                        Model imgui COMPILE_FEATURES cxx_std_17)

#Phase 4 : Command System(Undo / Redo, Command History)
                    config_lib(
                        Commands PRIVATE_SOURCES Commands /
                        CommandFactory.cpp Commands /
                        ExtendedCommandManager.cpp Commands /
                        ColorChangeCommand.cpp Commands /
                        CreateFigureCommand.cpp Commands /
                        DeleteFiguresCommand.cpp Commands /
                        DuplicateFigureCommand.cpp Commands /
                        GroupFiguresCommand.cpp Commands /
                        MacroCommand.cpp Commands /
                        MoveFigureCommand.cpp Commands /
                        PropertyChangeCommand.cpp Commands /
                        RotateFigureCommand.cpp Commands /
                        ScaleFigureCommand.cpp Commands /
                        UngroupFiguresCommand.cpp Commands / ImGUI /
                        CommandHistoryPanel.cpp PUBLIC_SOURCES Commands /
                        ICommand.hpp Commands / CommandFactory.hpp Commands /
                        ExtendedCommandManager.hpp Commands /
                        ColorChangeCommand.hpp Commands /
                        CreateFigureCommand.hpp Commands /
                        DeleteFiguresCommand.hpp Commands /
                        DuplicateFigureCommand.hpp Commands /
                        GroupFiguresCommand.hpp Commands /
                        MacroCommand.hpp Commands /
                        MoveFigureCommand.hpp Commands /
                        PropertyChangeCommand.hpp Commands /
                        RotateFigureCommand.hpp Commands /
                        ScaleFigureCommand.hpp Commands /
                        UngroupFiguresCommand.hpp Commands / ImGUI /
                        CommandHistoryPanel.hpp PUBLIC_DEPENDENCIES Model
                            UIFSMAdapter imgui PRIVATE_DEPENDENCIES
                                nlohmann_json::nlohmann_json spdlog::spdlog
                                    COMPILE_FEATURES cxx_std_17)

#Phase 5 : Navigation &Views System(Zoom, Pan, View Presets)
                        config_lib(
                            Navigation PRIVATE_SOURCES Navigation /
                            NavigationManager.cpp Navigation /
                            NavigationEventHandler.cpp ImGUI /
                            ViewPresetsPanel.cpp PUBLIC_SOURCES Navigation /
                            NavigationTypes.hpp Navigation /
                            NavigationManager.hpp Navigation /
                            NavigationEventHandler.hpp ImGUI /
                            ViewPresetsPanel.hpp PUBLIC_DEPENDENCIES
                                UIFSMAdapter CameraController ObjectManagement
                                    imgui PRIVATE_DEPENDENCIES spdlog::spdlog
                                        COMPILE_FEATURES cxx_std_17)

#Phase 6 : Precision &Snapping(Grid, Snap, Measurement)
                            config_lib(
                                Precision PRIVATE_SOURCES Precision /
                                GridManager.cpp Precision /
                                SnapManager.cpp Precision /
                                MeasurementManager.cpp Precision /
                                CoordinateInputManager.cpp Precision /
                                GridSettingsPanel.cpp Precision /
                                SnapSettingsPanel.cpp Precision /
                                CoordinateInputWidget.cpp Precision /
                                MeasurementDisplay.cpp Precision /
                                ExpressionEvaluator.cpp PUBLIC_SOURCES
                                    Precision /
                                GridManager.hpp Precision /
                                SnapManager.hpp Precision /
                                MeasurementManager.hpp Precision /
                                CoordinateInputManager.hpp Precision /
                                GridSettingsPanel.hpp Precision /
                                SnapSettingsPanel.hpp Precision /
                                CoordinateInputWidget.hpp Precision /
                                MeasurementDisplay.hpp Precision /
                                ExpressionEvaluator.hpp PUBLIC_DEPENDENCIES
                                    UIFSMAdapter glm::glm imgui tinyexpr
                                        COMPILE_FEATURES cxx_std_17)

# Phase 7: Polish & Optimization (Keyboard Shortcuts, Context Menus, Settings Dialog, Theme System, Tooltips, Help System, Performance Monitoring)
                                config_lib(
                                    Polish PRIVATE_SOURCES Polish /
                                    ShortcutManager.cpp Polish /
                                    ContextMenuManager.cpp Polish /
                                    SettingsDialog.cpp Polish /
                                    ThemeManager.cpp Polish /
                                    TooltipManager.cpp Polish /
                                    HelpBrowser.cpp PUBLIC_SOURCES Polish /
                                    ShortcutManager.hpp Polish /
                                    ContextMenuManager.hpp Polish /
                                    SettingsDialog.hpp Polish /
                                    ThemeManager.hpp Polish /
                                    TooltipManager.hpp Polish /
                                    HelpBrowser.hpp PUBLIC_DEPENDENCIES
                                        UIFSMAdapter ObjectManagement glm::glm imgui spdlog::spdlog COMPILE_FEATURES
                                            cxx_std_17)

#Select specific view
                                    add_subdirectory("OpenGL")

                                        add_library(View ALIAS ImGUIOpenGLView)