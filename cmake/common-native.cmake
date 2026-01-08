if(DEFINED ENV{RENODE_ROOT})
  message(STATUS "Renode root: $ENV{RENODE_ROOT}")
  set(renode_root $ENV{RENODE_ROOT})
else()
  message(FATAL_ERROR
    "Please set the environment variable RENODE_ROOT value to"
    "absolute path to the Renode root directory.")
endif()

set(renode_bridge_native ${renode_root}/src/Plugins/SystemCPlugin/SystemCNativeModule)
add_subdirectory(${renode_bridge_native} renode_bridge_native_build)
