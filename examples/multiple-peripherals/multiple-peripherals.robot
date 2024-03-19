*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/multiple-peripherals.resc
${UART}                             sysbus.usart1

*** Test Cases ***
Should Run The Multiple Peripherals Example
    Execute Script                  ${SCRIPT_PATH}
    Create Terminal Tester          ${UART}
    Start Emulation

    Wait For Line On Uart           Example complete!
