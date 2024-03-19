cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_examples)

file(GLOB ALL_EXAMPLES examples/*)
foreach(example ${ALL_EXAMPLES})
    if(IS_DIRECTORY ${example})
        set(OVERRIDE_OUTPUT_DIR ${example}/bin)
        add_subdirectory(${example})
    endif()
endforeach()
