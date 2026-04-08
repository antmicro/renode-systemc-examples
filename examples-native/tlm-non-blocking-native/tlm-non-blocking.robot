*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/tlm-non-blocking.resc
${VALUE}                            0xdeadbeef

*** Test Cases ***
Should Perform Non Blocking Transaction
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    Execute Command                 systemc WriteDoubleWord 0x0 ${VALUE}
    ${res}=                         Execute Command  systemc ReadDoubleWord 0x0
    Should Be Equal As Numbers      ${res}  ${VALUE}
