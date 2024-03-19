if(NOT DEFINED OVERRIDE_OUTPUT_DIR)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OVERRIDE_OUTPUT_DIR})
endif()

if(NOT USER_RENODE_DIR AND DEFINED ENV{RENODE_ROOT})
  message(STATUS "Using RENODE_ROOT from environment as USER_RENODE_DIR")
  set(USER_RENODE_DIR $ENV{RENODE_ROOT} CACHE PATH "Absolute path to Renode root directory or any other that contains SystemCModule.")
else()
  set(USER_RENODE_DIR CACHE PATH "Absolute path to Renode root directory or any other that contains SystemCModule.")
endif()

# Find path to SystemCModule in Renode distribution to link against it.
if(NOT SYSCMODULE_DIR)
  if(NOT USER_RENODE_DIR OR NOT IS_ABSOLUTE "${USER_RENODE_DIR}")
      message(FATAL_ERROR "Please set the CMake's USER_RENODE_DIR variable to an absolute path to Renode root directory or any other that contains SystemCModule.\nPass the '-DUSER_RENODE_DIR=<ABSOLUTE_PATH>' switch if you configure with the 'cmake' command. Optionally, consider using 'ccmake' or 'cmake-gui' which make it easier.")
  endif()

  message(STATUS "Looking for Renode SystemCModule inside ${USER_RENODE_DIR}...")

  file(GLOB_RECURSE SYSCMODULE_FOUND ${USER_RENODE_DIR}*/renode_bridge.h)
  list(LENGTH SYSCMODULE_FOUND SYSCMODULE_FOUND_N)
  if(${SYSCMODULE_FOUND_N} EQUAL 0)
      message(FATAL_ERROR "Couldn't find valid SystemCPlugin inside USER_RENODE_DIR!")
  elseif(${SYSCMODULE_FOUND_N} GREATER 1)
      string(REPLACE "/include/renode_bridge.h" "" ALL_FOUND ${SYSCMODULE_FOUND})
      message(FATAL_ERROR "Found more than one directory with SystemCPlugin inside USER_RENODE_DIR. Please choose one of them: ${SYSCMODULE_FOUND}")
  endif()

  string(REPLACE "/include/renode_bridge.h" "" SYSCMODULE_DIR ${SYSCMODULE_FOUND})

  message(STATUS "SystemCModule found: ${SYSCMODULE_DIR}")

  set(SYSCMODULE_DIR ${SYSCMODULE_DIR} CACHE INTERNAL "")
endif()

find_library(SYSTEMC_LIB
    NAMES systemc libsystemc
)
message(STATUS "Found SystemC: ${SYSTEMC_LIB}")

if (NOT TARGET renode_bridge)
    add_subdirectory(${SYSCMODULE_DIR} renode_bridge)
endif()

file(GLOB_RECURSE FOUND_SOURCES
    systemc/src/*.cpp)
set(SOURCES ${SOURCES} ${FOUND_SOURCES})
set(INCLUDE_DIRS ${INCLUDE_DIRS} systemc/include ${SYSCMODULE_DIR}/include)

add_executable(${EXAMPLE_NAME} ${SOURCES})
target_include_directories(${EXAMPLE_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${EXAMPLE_NAME} ${SYSTEMC_LIB} renode_bridge)
