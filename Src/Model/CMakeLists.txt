cmake_minimum_required (VERSION 3.23)
project (${PROJECT_NAME})


add_library (FlatFigureModel)
add_dependencies (${PROJECT_NAME} FlatFigureModel)

target_sources (FlatFigureModel
    PRIVATE
        FlatFigure.cpp
    PUBLIC
        FILE_SET flatFigureModelHeaders
        TYPE HEADERS
        BASE_DIRS ../
        FILES
            FlatFigure.hpp
)

target_compile_features (FlatFigureModel
    PUBLIC
        cxx_std_17
)

install (TARGETS FlatFigureModel FILE_SET flatFigureModelHeaders)

add_library (model ALIAS FlatFigureModel)