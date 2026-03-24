*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/cortex-m-signals.resc

${SIGNAL_NON_MASKABLE_INTERRUPT}    1000
${SIGNAL_CORE_RESET_IN}             1001
${SIGNAL_CPU_WAIT}                  1002
${SIGNAL_POWER_ON_RESET}            1005
${SIGNAL_SYSTEM_RESET_REQUEST}      1006

${TRIGGER_SIGNAL_OFFSET}            0x100

${DUMMY_CPU_PC}                     0xdeadbeee

*** Keywords ***
Create Machine
    Execute Script                  ${SCRIPT_PATH}
    Create Log Tester               1

    # So we can see the GPIOs being activated.
    Execute Command                 logLevel -1 signals

SystemC Peripheral Should Return
    [Arguments]                     ${value}  ${offset}  ${message}
    ${res}=                         Execute Command  signals ReadByte ${offset}
    Should Be Equal As Integers     ${res}  ${value}  ${message}

Write ${value} To SystemC Peripheral Offset ${offset}
    Execute Command                 signals WriteDoubleWord ${offset} ${value}

Trigger SystemC Signal ${signal}
    Write ${signal} To SystemC Peripheral Offset ${TRIGGER_SIGNAL_OFFSET}

SystemC Signal ${signal} Should Be ${state:(Set|Unset)}
    [Arguments]                     ${message}
    ${expected_value}=              Evaluate  int($state == "Set")
    SystemC Peripheral Should Return  ${expected_value}  ${signal}  ${message}

Run Renode Command
    [Arguments]                     ${command}  ${prefix}=${EMPTY}
    ${raw}=                         Execute Command  ${command}
    ${result}=                      Evaluate  $raw.strip()
    Execute Command                 machine InfoLog "${prefix} - ${command} = ${result}"
    RETURN                          ${result}

Modify CPU Peripheral State
    Execute Command                 cpu PC ${DUMMY_CPU_PC}
    Execute Command                 nvic IRQ Set
    Execute Command                 dwt WriteDoubleWord 0x0 0x1

    # Before triggering the signal, these state changes should have happened.
    ${pc_before}=                   Run Renode Command  cpu PC  prefix=before
    ${nvic_irq_before}=             Run Renode Command  nvic IRQ IsSet  prefix=before
    ${dwt_counter_enabled_before}=  Run Renode Command  dwt ReadDoubleWord 0x0  prefix=before
    Should Be Equal As Integers     ${pc_before}  ${DUMMY_CPU_PC}  The PC should've been initialized
    Should Be Equal                 ${nvic_irq_before}  True  The NVIC IRQ should've been high
    Should Be Equal As Integers     ${dwt_counter_enabled_before}  0x1  The DWT's cycle counter should have been enabled

CPU Peripherals Should Have Reset
    ${pc_after}=                    Run Renode Command  cpu PC  prefix=after
    ${nvic_irq_after}=              Run Renode Command  nvic IRQ IsSet  prefix=after
    ${dwt_counter_enabled_after}=   Run Renode Command  dwt ReadDoubleWord 0x0  prefix=after

    Should Not Be Equal As Integers  ${pc_after}  ${DUMMY_CPU_PC}
    ...                             message=The CPU should have reset, but its PC remained the same
    Should Be Equal                  ${nvic_irq_after}  False  The NVIC should have reset, but its IRQ remained high
    Should Not Be Equal As Integers  ${dwt_counter_enabled_after}  0x0
    ...                             message=The DWT should have reset, but its cycle counter remained enabled

Wait For SystemC Signal ${signal}
    [Arguments]                     ${value}=True
    # Wait until we have received the signal back from SystemC and handled it.
    Wait For Log Entry              SystemC-triggered GPIO ${signal}, value ${value}

*** Test Cases ***
Should Raise Cpu Wait Signal
    Create Machine

    ${cpu_wait_before}=             Run Renode Command  cpu CpuWaitSignal IsSet
    Should Be Equal                 ${cpu_wait_before}  False  message=CPUWAIT should start off low

    Trigger SystemC Signal ${SIGNAL_CPU_WAIT}

    # Wait for the signal before checking CPU state, so it has time to update.
    Wait For SystemC Signal ${SIGNAL_CPU_WAIT}
    ${cpu_wait_after}=              Run Renode Command  cpu CpuWaitSignal IsSet
    Should Not Be Equal             ${cpu_wait_before}  ${cpu_wait_after}  message=CPUWAIT signal should've been raised

Raising Power-On Reset Signal Should Reset CPU Peripherals
    Create Machine

    # Make some changes to state, so we can see if it gets reset properly.
    Modify CPU Peripheral State

    Trigger SystemC Signal ${SIGNAL_POWER_ON_RESET}

    # Wait for the signal before checking CPU state, so it has time to reset.
    Wait For SystemC Signal ${SIGNAL_POWER_ON_RESET}
    CPU Peripherals Should Have Reset

Raising Core Reset Signal Should Reset CPU Peripherals
    Create Machine

    # Make some changes to state, so we can see if it gets reset properly.
    Modify CPU Peripheral State

    Trigger SystemC Signal ${SIGNAL_CORE_RESET_IN}

    # Wait for the signal before checking CPU state, so it has time to reset.
    Wait For SystemC Signal ${SIGNAL_CORE_RESET_IN}
    CPU Peripherals Should Have Reset

Should Trigger NMI
    Create Machine

    # So we can see the NVIC getting activated.
    Execute Command                 logLevel 0 nvic

    Trigger SystemC Signal ${SIGNAL_NON_MASKABLE_INTERRUPT}

    # Check NVIC IRQ before checking for signal,
    # because the signal gets logged after its events have been handled.
    Wait For Log Entry              Set pending IRQ NMI
    Wait For SystemC Signal ${SIGNAL_NON_MASKABLE_INTERRUPT}

Should Receive System Reset Request Signal
    Create Machine

    SystemC Signal ${SIGNAL_SYSTEM_RESET_REQUEST} Should Be Unset  message=SysResetReq should initially be low

    Execute Command                 nvic SystemResetRequest Set True

    SystemC Signal ${SIGNAL_SYSTEM_RESET_REQUEST} Should Be Set  message=SysResetReq should have gone high

Should Trigger NVIC IRQs
    Create Machine
    Create Log Tester               1

    # So we can see the IRQs getting triggered.
    Execute Command                 logLevel 0 nvic

    FOR  ${irq}  IN RANGE  480
        Trigger SystemC Signal ${irq}
        Wait For Log Entry              nvic: Set pending IRQ HardwareIRQ#${irq}
    END
