add_executable(systemc_example
    virtual_platform.cpp
    peripheral.cpp
    testcase.cpp
)
target_link_libraries(systemc_example systemc)