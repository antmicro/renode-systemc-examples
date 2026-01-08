cmake_minimum_required(VERSION 3.18.4)

project(renode_systemc_timesync_native_example)

set(EXAMPLE_NAME timesync_native)

add_library(timesync_native SHARED
  systemc/src/main.cpp
  systemc/src/timekeeper.cpp
)

target_include_directories(timesync_native PUBLIC
  systemc/include
)
target_link_libraries(timesync_native PUBLIC systemc renode_bridge_native)

include(../../cmake/common-native.cmake)
