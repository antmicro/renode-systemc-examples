cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_examples)

file(GLOB ALL_EXAMPLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*/)
foreach(example ${ALL_EXAMPLES})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${example}/)
        add_subdirectory(${example} ${CMAKE_BINARY_DIR}/${example})
    endif()
endforeach()

file(GLOB ALL_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*/)
foreach(test ${ALL_TESTS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test})
        add_subdirectory(${test} ${CMAKE_BINARY_DIR}/${test})
    endif()
endforeach()

file(GLOB ALL_EXAMPLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples-native/*/)
foreach(example ${ALL_EXAMPLES})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${example})
        add_subdirectory(${example} ${CMAKE_BINARY_DIR}/${example})
    endif()
endforeach()
