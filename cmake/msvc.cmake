if (MSVC)
    target_compile_options(${EXAMPLE_NAME} PRIVATE "/vmg")
    target_link_options(${EXAMPLE_NAME} PRIVATE "/ignore:4099")
endif()
