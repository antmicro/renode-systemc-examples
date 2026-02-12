if(DEFINED USER_SYSTEMC_LIB_DIR)
    message(STATUS "Looking for SystemC library in: ${USER_SYSTEMC_LIB_DIR}")
    find_library(systemc_library PATHS ${USER_SYSTEMC_LIB_DIR} NAMES systemc libsystemc)
else()
    message(WARNING "No USER_SYSTEMC_LIB_DIR specified - attempting to use system-wide SystemC installation.")
    find_library(systemc_library NAMES systemc libsystemc)
endif()

add_library(systemc UNKNOWN IMPORTED)
set_target_properties(systemc PROPERTIES IMPORTED_LOCATION "${systemc_library}")

if(DEFINED USER_SYSTEMC_INCLUDE_DIR)
    target_include_directories(${EXAMPLE_NAME} PUBLIC ${USER_SYSTEMC_INCLUDE_DIR})
    set_target_properties(systemc PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${USER_SYSTEMC_INCLUDE_DIR}")
else()
    message(WARNING "No USER_SYSTEMC_INCLUDE_DIR specified - headers will only be included from standard locations.")
endif()

target_link_libraries(${EXAMPLE_NAME} PRIVATE systemc)
