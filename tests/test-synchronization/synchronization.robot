*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/synchronization.resc

*** Test Cases ***
Should Not Deadlock Writing To ExecuteInLockPeripheral
    Execute Script                  ${SCRIPT_PATH}
    Create Log Tester               1
    Start Emulation

    Wait For Log Entry              Got write request with value 0xAB
