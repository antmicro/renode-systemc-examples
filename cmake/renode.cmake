if(DEFINED USER_RENODE_DIR)
    set(renode_root ${USER_RENODE_DIR})
elseif(DEFINED ENV{RENODE_ROOT})
    set(renode_root $ENV{RENODE_ROOT})
else()
    message(FATAL_ERROR "Please set the CMake's USER_RENODE_DIR variable to an absolute path to "
    "Renode root directory or any other that contains SystemCModule.\nPass the "
    "'-DUSER_RENODE_DIR=<ABSOLUTE_PATH>' switch if you configure with the 'cmake' command. "
    "Optionally, consider using 'ccmake' or 'cmake-gui' which make it easier.")
endif()

if(EXISTS ${renode_root})
    message(STATUS "Found Renode: ${renode_root}")
else()
    message(FATAL_ERROR "Path doesn't exist: ${renode_root}!")
endif()

set(systemc_plugin_path ${renode_root}/plugins/SystemCModule)
set(systemc_plugin_src_path ${renode_root}/src/Plugins/SystemCPlugin/SystemCModule)
if(EXISTS ${systemc_plugin_path})
    message(STATUS "Found SystemCPlugin: ${systemc_plugin_path}")
    set(systemc_plugin ${systemc_plugin_path})
elseif(EXISTS ${systemc_plugin_src_path})
    message(STATUS "Found SystemCPlugin: ${systemc_plugin_src_path}")
    set(systemc_plugin ${systemc_plugin_src_path})
else()
    message(FATAL_ERROR "Couldn't find SystemCPlugin at: ${renode_root}!")
endif()

if(NOT TARGET renode_bridge)
    add_subdirectory(${systemc_plugin} ${CMAKE_BINARY_DIR}/SystemCModule)
endif()
