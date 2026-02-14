# TinyExpr - Tiny recursive descent expression parser and evaluator
cmake_minimum_required(VERSION 3.19)

# Create TinyExpr as a STATIC library with source files
add_library(tinyexpr STATIC
    tinyexpr.c
)

# Set the include directory for TinyExpr (PUBLIC so consumers get it)
target_include_directories(tinyexpr PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
