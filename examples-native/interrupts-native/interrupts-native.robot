*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/interrupts.resc
${UART}                             sysbus.usart1

*** Test Cases ***
Should Invoke Interrupt Handlers Initiated By SystemC
    Execute Script                  ${SCRIPT_PATH}
    Create Terminal Tester          ${UART}
    Start Emulation

    Wait For Line On Uart           Interrupt handler for interrupter 0 (every 1 second)
    Wait For Line On Uart           Interrupt handler for interrupter 1 (every 3 seconds)
