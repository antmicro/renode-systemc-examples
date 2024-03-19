*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/direct-connection.resc

*** Keywords ***
Read Value Should Be Equal To
    [Arguments]                     ${address}  ${value}
    ${res}=                         Execute Command  sysbus ReadByte ${address}
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Transfer Data Using Direct Connections
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    # Set value stored in the first SystemC peripheral to 42
    Execute Command                 sysbus WriteDoubleWord 0x9000000 42

    # Set value stored in the second SystemC peripheral to 43
    Execute Command                 sysbus WriteDoubleWord 0x8000000 43

    # Each peripheral should return value stored in the other one,
    # obtaining it through a direct connection "behind the scenes"
    Read Value Should Be Equal To   0x8000000  42
