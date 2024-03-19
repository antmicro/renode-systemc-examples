*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/systemc_renode_lt.resc

*** Keywords ***
Memory Should Be Equal To
    [Arguments]                     ${address}  ${value}
    ${res}=                         Execute Command  sysbus ReadDoubleWord ${address}
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Write To And Read From "at_target_1_phase" Module
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    Execute Command                 sysbus WriteDoubleWord 0x100000000 0x0
    Memory Should Be Equal To       0x100000000  0x0
    Execute Command                 sysbus WriteDoubleWord 0x100000004 0x1
    Memory Should Be Equal To       0x100000004  0x1
    Execute Command                 sysbus WriteDoubleWord 0x100000008 0x2
    Memory Should Be Equal To       0x100000008  0x2
    Execute Command                 sysbus WriteDoubleWord 0x10000000C 0x3
    Memory Should Be Equal To       0x10000000C  0x3
    Execute Command                 sysbus WriteDoubleWord 0x100000010 0x4
    Memory Should Be Equal To       0x100000010  0x4

Should Write To And Read From "lt_target" Module
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    Execute Command                 sysbus WriteDoubleWord 0x110000000 0x0
    Memory Should Be Equal To       0x110000000  0x0
    Execute Command                 sysbus WriteDoubleWord 0x110000004 0x1
    Memory Should Be Equal To       0x110000004  0x1
    Execute Command                 sysbus WriteDoubleWord 0x110000008 0x2
    Memory Should Be Equal To       0x110000008  0x2
    Execute Command                 sysbus WriteDoubleWord 0x11000000C 0x3
    Memory Should Be Equal To       0x11000000C  0x3
    Execute Command                 sysbus WriteDoubleWord 0x110000010 0x4
