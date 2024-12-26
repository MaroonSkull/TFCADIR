cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})



config_lib(FlatFigureModel
    BASE_DIR
        ../
    PRIVATE_SOURCES
        FlatFigure.cpp
    PUBLIC_SOURCES
        FlatFigure.hpp
        Model.hpp
    PRIVATE_DEPENDENCIES
        spdlog::spdlog
    COMPILE_FEATURES
        cxx_std_17
)

add_library (Model ALIAS FlatFigureModel)