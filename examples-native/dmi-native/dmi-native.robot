*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/dmi_test.resc

*** Keywords ***
Memory Should Be Equal To
    [Arguments]                     ${address}  ${value}
    ${res}=                         Execute Command  sysbus ReadDoubleWord ${address}
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Perform Memory-To-Memory Transfer
    Execute Script                  ${SCRIPT_PATH}
    ${done}=                        Create LED Tester  sysbus.done
    ${io_access}=                   Create LED Tester  sysbus.unexpected_io_access
    Start Emulation

    # Initialize memory
    Execute Command                 sysbus WriteDoubleWord 0x20001230 0x00000000
    Execute Command                 sysbus WriteDoubleWord 0x20002468 0x00000000

    # Signal dmi_test peripheral to begin the test
    Execute Command                 sysbus.dmi_test OnGPIO 1 True

    # Execute code from a DMI-enabled SystemC memory region. The first fetch
    # goes through TLM, then tlib maps the DMI region and the store below uses
    # the mapped softMMU path instead of calling back into SystemC b_transport.
    Execute Command                 cpu PC 0x09000000
    Execute Command                 cpu IsHalted false

    Assert LED State                true  timeout=1  testerId=${done}
    Memory Should Be Equal To       0x20001230  0x0b1e55ed
    Memory Should Be Equal To       0x20002468  0x00c0ffee
    # The CPU's accesses should not have gone through the I/O path...
    Assert LED State                false  timeout=0.1  testerId=${io_access}
    # ... but this one will, hence why we only do it after checking the LED state :)
    Memory Should Be Equal To       0x09000080  0x87654321
    Assert LED State                true  timeout=0  testerId=${io_access}
