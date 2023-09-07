cmake_minimum_required (VERSION 3.8)
project (${PROJECT_NAME})


add_library (model STATIC flatFigureModel.cpp)
add_dependencies (${PROJECT_NAME} model)

target_include_directories (model PUBLIC ./)

target_compile_features (model PUBLIC cxx_std_17)