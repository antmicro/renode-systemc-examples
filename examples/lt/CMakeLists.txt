cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_lt_example)

set(EXAMPLE_NAME lt)

include(../../cmake/common.cmake)

# additional sources for the example
file(GLOB_RECURSE sources systemc/third-party/systemc-lt-example/*.cpp)
target_sources(lt PRIVATE ${source})
target_include_directories(lt PRIVATE systemc/third-party/systemc-lt-example)
