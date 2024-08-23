*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/gpio-connection.resc

*** Keywords ***
SystemC Peripheral Should Return
    [Arguments]                     ${value}
    ${res}=                         Execute Command  systemc ReadByte 0x0
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Communicate through GPIO
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    # SystemC peripheral returns 0 on read until it sees a GPIO event
    SystemC Peripheral Should Return  0

    # Initialize GPIO peripheral: enable GPIO 1 as output
    Execute Command                 gpio WriteDoubleWord 0x4 0x1

    # Set GPIO #1
    Execute Command                 gpio WriteDoubleWord 0xA0 0x2

    # SystemC peripheral should now return 1 as it as received a GPIO event
    SystemC Peripheral Should Return  1
