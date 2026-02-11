cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_examples)

file(GLOB ALL_EXAMPLES examples/*)
foreach(example ${ALL_EXAMPLES})
    if(IS_DIRECTORY ${example})
        add_subdirectory(${example} ${CMAKE_BINARY_DIR}/${example})
    endif()
endforeach()

file(GLOB ALL_TESTS tests/*)
foreach(test ${ALL_TESTS})
    if(IS_DIRECTORY ${test})
        add_subdirectory(${test} ${CMAKE_BINARY_DIR}/${test})
    endif()
endforeach()
