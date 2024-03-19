*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/transaction-delay.resc
${UART}                             sysbus.usart1

*** Keywords ***
Virtual Time Should Be Equal To
    [Arguments]                     ${time_string}
    ${res}=                         Execute Command  machine ElapsedVirtualTime
    Should Contain                  ${res}  Elapsed Virtual Time: ${time_string}

*** Test Cases ***
Should Respect SystemC Transaction Durations
    Execute Script                  ${SCRIPT_PATH}
    Create Terminal Tester          ${UART}
    Start Emulation

    Wait For Line On Uart           SystemC virtual time (1s transaction delay): 1 s
    Virtual Time Should Be Equal To  00:00:02

    Wait For Line On Uart           SystemC virtual time (1s transaction delay): 2 s
    Virtual Time Should Be Equal To  00:00:03

    Wait For Line On Uart           SystemC virtual time (1s transaction delay): 6 s
    Virtual Time Should Be Equal To  00:00:07
