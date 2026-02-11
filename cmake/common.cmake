list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake")

file(GLOB_RECURSE systemc_sources systemc/src/*.cpp)
add_executable(${EXAMPLE_NAME} ${systemc_sources})

target_include_directories(${EXAMPLE_NAME} PUBLIC
    ${systemc_plugin}/include
    systemc/include
)

target_link_libraries(${EXAMPLE_NAME} PRIVATE renode_bridge)

include(systemc)
include(msvc)
include(renode)
