file(GLOB_RECURSE systemc_sources systemc/src/*.cpp)

if(BUILD_SHARED)
    add_library(${EXAMPLE_NAME} SHARED ${systemc_sources})
    target_link_libraries(${EXAMPLE_NAME} PUBLIC renode_bridge_native)
else()
    add_executable(${EXAMPLE_NAME} ${systemc_sources})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE renode_bridge)
endif()

target_include_directories(${EXAMPLE_NAME} PUBLIC
    ${systemc_plugin}/include
    systemc/include
)

include(../../cmake/systemc.cmake)
include(../../cmake/msvc.cmake)
include(../../cmake/renode.cmake)
