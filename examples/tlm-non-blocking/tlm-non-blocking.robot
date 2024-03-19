*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/tlm-non-blocking.resc

*** Keywords ***
Read Value Should Be Equal To
    [Arguments]                     ${address}  ${value}
    ${res}=                         Execute Command  sysbus ReadByte ${address}
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Transfer Data Correctly In All Directions:
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    # This will set internal values of systemc_peripheral0 and 1 to 42 and 43 respectively.
    Execute Command                 sysbus WriteDoubleWord 0x9000000 42
    Execute Command                 sysbus WriteDoubleWord 0x8000000 43

    # This will cause systemc_peripheral0 to write it's internal value (42) to memory
    # at address 0x20000000
    Execute Command                 sysbus WriteDoubleWord 0x9000010 0x20000000
    Read Value Should Be Equal To   0x20000000  42

    # This will cause systemc_peripheral1 to write it's internal value (43) to memory
    # at address 0x20000010
    Execute Command                 sysbus WriteDoubleWord 0x8000010 0x20000010
    Read Value Should Be Equal To   0x20000010  43

    # This will cause systemc_peripheral0 to use direct connection to return the internal value
    # from systemc_peripheral1 (43)
    Read Value Should Be Equal To   0x9000000  43

    # This will cause systemc_peripheral1 to use direct connection to return the internal value
    # from systemc_peripheral0 (42)
    Read Value Should Be Equal To   0x8000000  42
