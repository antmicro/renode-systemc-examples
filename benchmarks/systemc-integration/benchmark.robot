*** Settings ***
Library           Process

*** Variables ***
${ITERATIONS}     100000
${PERIPHERALS}    1

*** Keywords ***
Run Workload
    Execute Command     $iterations=${ITERATIONS}
    Execute Command     $peripherals=${PERIPHERALS}
    Execute Script      ${CURDIR}/renode/workload.resc
    Create Log Tester   120
    Start Emulation
    ${measurement}=     Wait For Log Entry  Time measurement finished  timeout=120  pauseEmulation=true
    Log To Console      ${measurement}
    ${errors}=          Execute Command  sysbus ReadDoubleWord 0x10001000
    Should Be Equal As Numbers  ${errors}  0

*** Test Cases ***
Built-In Renode Trivial Peripheral
    [Tags]              renode
    Execute Command     mach create "peripheral-profile"
    Execute Command     machine LoadPlatformDescription @${CURDIR}/renode/renode.repl
    Run Workload

Native SystemC Trivial Peripheral
    [Tags]              systemc
    [Timeout]           120 seconds
    ${result}=          Run Process
    ...                 ${CURDIR}/build/bin/systemc_trivial
    ...                 ${CURDIR}/renode/systemc.resc
    ...                 ${ITERATIONS}
    ...                 ${PERIPHERALS}
    ...                 stderr=STDOUT
    ...                 timeout=120s
    ...                 on_timeout=kill
    Log To Console      ${result.stdout}
    Should Be Equal As Integers  ${result.rc}  0
    Should Contain      ${result.stdout}  Time measurement finished
