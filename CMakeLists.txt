cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_examples)

file(GLOB ALL_EXAMPLES examples/*)
foreach(example ${ALL_EXAMPLES})
    if(IS_DIRECTORY ${example})
        set(OVERRIDE_OUTPUT_DIR ${example}/bin)
        add_subdirectory(${example})
    endif()
endforeach()

file(GLOB ALL_TESTS tests/*)
foreach(test ${ALL_TESTS})
    if(IS_DIRECTORY ${test})
        set(OVERRIDE_OUTPUT_DIR ${test}/bin)
        add_subdirectory(${test})
    endif()
endforeach()
