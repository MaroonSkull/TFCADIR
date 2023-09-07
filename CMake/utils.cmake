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
