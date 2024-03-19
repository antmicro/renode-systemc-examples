cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_lt_example)

set(EXAMPLE_NAME lt)

file(GLOB_RECURSE SOURCES
    systemc/third-party/systemc-lt-example/*.cpp)
set(INCLUDE_DIRS systemc/third-party/systemc-lt-example)
include(../../cmake/common.cmake)
