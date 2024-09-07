function(add_subdirectory_checked dir)
    if (NOT EXISTS ${dir}/CMakeLists.txt)
        message(FATAL_ERROR
            "Error: Subdirectory " ${dir} " not found\n"
            "You may have cloned the repository without the --recursive flag\n"
            "Use `git submodule update --init --recursive` to get the required dependencies"
        )
    endif ()
    add_subdirectory(${dir})
endfunction()

# config_lib(
#     <lib_name>
#     <PUBLIC_SOURCES <public sources> ...>
#     [INTERFACE]
#     [BASE_DIR <base directory>]
#     [PRIVATE_SOURCES <private sources> ...]
#     [PUBLIC_DEPENDENCIES <public dependencies> ...]
#     [PRIVATE_DEPENDENCIES <private dependencies> ...]
#     [INTERFACE_DEPENDENCIES <interface dependencies> ...]
#     [COMPILE_FEATURES <compile features> ...]
# )
#
# lib_name: name of the library
# PUBLIC_SOURCES: list of public sources
# INTERFACE: is it interface library
# BASE_DIR: include directory
# PRIVATE_SOURCES: list of private sources
# PUBLIC_DEPENDENCIES: list of public dependencies
# PRIVATE_DEPENDENCIES: list of private dependencies
# INTERFACE_DEPENDENCIES: list of interface dependencies
# COMPILE_FEATURES: list of compile features
#
# The library is created with the name lib_name.
# lib_name, PUBLIC_SOURCES are required.
# BASE_DIR, PRIVATE_SOURCES, PUBLIC_DEPENDENCIES and COMPILE_FEATURES are optional.
# The library is added to the target PROJECT_NAME.
#
# Example:
# config_lib(ImGUI PRIVATE_SOURCES ImGUI.cpp PUBLIC_SOURCES ImGUI.hpp PUBLIC_DEPENDENCIES imgui)
function(config_lib lib_name)
    set(pref config_lib)
    set(options INTERFACE)
    set(oneValueArgs BASE_DIR)
    set(multiValueArgs PRIVATE_SOURCES PUBLIC_SOURCES PUBLIC_DEPENDENCIES PRIVATE_DEPENDENCIES INTERFACE_DEPENDENCIES COMPILE_FEATURES)

    cmake_parse_arguments(PARSE_ARGV 1 "${pref}" "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(lib_headers_name "the${lib_name}Headers")
    set(base_dir ${${pref}_BASE_DIR})
    set(private_sources ${${pref}_PRIVATE_SOURCES})
    set(public_sources ${${pref}_PUBLIC_SOURCES})
    set(public_dependencies ${${pref}_PUBLIC_DEPENDENCIES})
    set(private_dependencies ${${pref}_PRIVATE_DEPENDENCIES})
    set(interface_dependencies ${${pref}_INTERFACE_DEPENDENCIES})
    set(compile_features ${${pref}_COMPILE_FEATURES})
    if(${${pref}_INTERFACE})
        set(interface INTERFACE)
        set(visibility INTERFACE)
    else()
        set(interface STATIC)
        set(visibility PUBLIC)
    endif()


    if (NOT "${${pref}_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR
            "Unrecognized arguments: ${${pref}_UNPARSED_ARGUMENTS}"
        )
    endif ()


    if (lib_name STREQUAL "")
        message(FATAL_ERROR
            "lib_name must be provided"
        )
    endif ()

    add_library (${lib_name} ${interface})
    add_dependencies (${PROJECT_NAME} ${lib_name})

    if (private_sources AND NOT "${private_sources}" STREQUAL "")
        target_sources (${lib_name}
            PRIVATE
                ${private_sources}
        )
    endif ()

    # check if public sources and base dir are provided
    if (NOT "${public_sources}" STREQUAL "")
        if(NOT "${base_dir}" STREQUAL "")
            target_sources (${lib_name}
                PUBLIC
                    FILE_SET ${lib_headers_name}
                    TYPE HEADERS
                    BASE_DIRS ${base_dir}
                    FILES
                        ${public_sources}
            )
        else()
            target_sources (${lib_name}
                PUBLIC
                    FILE_SET ${lib_headers_name}
                    TYPE HEADERS
                    FILES
                        ${public_sources}
            )
        endif()
    else()
        message(FATAL_ERROR
            "No public_sources(${public_sources}) provided for ${lib_name}"
        )
    endif ()


    # if compile features are provided, set them
    if (NOT "${compile_features}" STREQUAL "")
        target_compile_features (${lib_name}
            ${visibility}
                ${compile_features}
        )
    endif ()

    # if dependencies are provided, and this is not INTERFACE library, link them
    if (NOT ${${pref}_INTERFACE})
        if (NOT "${public_dependencies}" STREQUAL "")
            target_link_libraries (${lib_name}
                PUBLIC
                    ${public_dependencies}
            )
        endif ()
        if (NOT "${private_dependencies}" STREQUAL "")
            target_link_libraries (${lib_name}
                PRIVATE
                    ${private_dependencies}
            )
        endif ()
    endif()
    if (NOT "${interface_dependencies}" STREQUAL "")
        target_link_libraries (${lib_name}
            INTERFACE
                ${interface_dependencies}
        )
    endif ()
    
    install (TARGETS ${lib_name} FILE_SET ${lib_headers_name})
endfunction(config_lib)
