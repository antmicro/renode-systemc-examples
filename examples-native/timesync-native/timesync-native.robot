*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/timesync.resc
${UART}                             sysbus.usart1

*** Keywords ***
Virtual Time Should Be Equal To
    [Arguments]                     ${time_string}
    ${res}=                         Execute Command  machine ElapsedVirtualTime
    Should Contain                  ${res}  Elapsed Virtual Time: ${time_string}

*** Test Cases ***
Should Synchronize Renode And SystemC Virtual Times
    Execute Script                  ${SCRIPT_PATH}
    Create Terminal Tester          ${UART}
    Start Emulation

    Wait For Line On Uart           SystemC virtual time: 1 s
    Virtual Time Should Be Equal To  00:00:01

    Wait For Line On Uart           SystemC virtual time: 2 s
    Virtual Time Should Be Equal To  00:00:02

    Wait For Line On Uart           SystemC virtual time: 5 s
    Virtual Time Should Be Equal To  00:00:05
