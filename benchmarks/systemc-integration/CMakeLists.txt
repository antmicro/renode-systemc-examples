cmake_minimum_required(VERSION 3.18.4)
project(systemc_b_transport_benchmark LANGUAGES CXX)

set(EXAMPLE_NAME systemc_trivial)
set(USE_RENODE_NATIVE_INTERFACE ON)

include(../../cmake/common.cmake)

set_target_properties(${EXAMPLE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)
