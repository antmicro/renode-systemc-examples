*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/cortex-m-signals.resc

${SIGNAL_NON_MASKABLE_INTERRUPT}    1000
${SIGNAL_CORE_RESET_IN}             1001
${SIGNAL_CPU_WAIT}                  1002
${SIGNAL_INIT_NS_VTOR}              1003
${SIGNAL_INIT_S_VTOR}               1004
${SIGNAL_POWER_ON_RESET}            1005
${SIGNAL_SYSTEM_RESET_REQUEST}      1006

${TRIGGER_SIGNAL_OFFSET}            0x100

${DUMMY_CPU_PC}                     0xdeadbeee

# These are arbitrary values that get set by SystemC when triggering INITSVTOR/INITNSVTOR
${VTOR_INITIAL_ADDRESS}             0x20000000
${VTOR_NON_SECURE_PORT_ADDRESS}     0x2000A000
${VTOR_PORT_ADDRESS}                0x2000B000

*** Keywords ***
Create Machine
    # The setup script is loading a Cortex-M55 with TrustZone enabled
    Execute Script                  ${SCRIPT_PATH}
    Create Log Tester               1

    # So we can see the GPIOs being activated.
    Execute Command                 logLevel -1 signals

Create Machine With Trust Zone Enabled
    Create Machine
    Execute Command                 logLevel -1 cpu

Create Machine With Trust Zone Disabled
    Execute Command                 mach create "cortex-m-signals-trust-zone"
    # Define a platform with a Cortex-M and with enabled Trust Zone
    ${base_platform}=               Get File  ${CURDIR}/renode/cortex-m-signals.repl
    ${platform}=                    Catenate  SEPARATOR=${\n}
    ...                             ${base_platform}
    ...                             cpu: { enableTrustZone: false }
    Execute Command                 machine LoadPlatformDescriptionFromString """${platform}"""
    Execute Command                 sysbus.signals SystemCExecutablePath @${CURDIR}/bin/cortex-m-signals

    Create Log Tester               1
    Execute Command                 logLevel -1 cpu
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
    Should Be Equal                 ${nvic_irq_after}  False  The NVIC should have reset, but its IRQ remained high
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

System Reset Request Signal Should Be Cleared On Reset
    Create Machine

    # Make some changes to state, so we can see if it gets reset properly.
    Modify CPU Peripheral State

    SystemC Signal ${SIGNAL_SYSTEM_RESET_REQUEST} Should Be Unset  message=SysResetReq should initially be low
    Execute Command                 nvic SystemResetRequest Set True
    SystemC Signal ${SIGNAL_SYSTEM_RESET_REQUEST} Should Be Set  message=SysResetReq should have gone high

    # SystemC should now raise the nSYSRESET signal
    Wait For SystemC Signal ${SIGNAL_CORE_RESET_IN}
    CPU Peripherals Should Have Reset

    # Even if the reset signal is still held high, we want the SYSRESETREQ to have been cleared.
    SystemC Signal ${SIGNAL_SYSTEM_RESET_REQUEST} Should Be Unset  message=SysResetReq should have been cleared

Should Trigger NVIC IRQs
    Create Machine

    # So we can see the IRQs getting triggered.
    Execute Command                 logLevel 0 nvic

    FOR  ${irq}  IN RANGE  480
        Trigger SystemC Signal ${irq}
        Wait For Log Entry              nvic: Set pending IRQ HardwareIRQ#${irq}
    END

SystemC Should Set Non Secure Vector Table Offset Register Using Non Secure Port With TrustZone Enabled
    Create Machine With Trust Zone Enabled

    Trigger SystemC Signal ${SIGNAL_INIT_NS_VTOR}
    Wait For Log Entry              signals: SystemC Non Secure Vector Table Offset: ${VTOR_NON_SECURE_PORT_ADDRESS}

    # VectorTableOffsetNonSecure should not yet have been updated
    ${vtor_non_secure}=             Execute Command  cpu VectorTableOffsetNonSecure
    Should Be Equal As Integers     ${vtor_non_secure}  ${VTOR_INITIAL_ADDRESS}

    Execute Command                 cpu Reset
    Wait For Log Entry              cpu: VectorTableOffset_NS set to ${VTOR_NON_SECURE_PORT_ADDRESS}

    # VectorTableOffsetNonSecure should have been updated out of reset
    ${vtor_non_secure}=             Execute Command  cpu VectorTableOffsetNonSecure
    Should Be Equal As Integers     ${vtor_non_secure}  ${VTOR_NON_SECURE_PORT_ADDRESS}

    # The secure offset should remain unchanged
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_INITIAL_ADDRESS}

SystemC Should Set Vector Table Offset Register Using Secure Port With TrustZone Enabled
    Create Machine With Trust Zone Enabled

    Trigger SystemC Signal ${SIGNAL_INIT_S_VTOR}
    Wait For Log Entry              signals: SystemC Vector Table Offset: ${VTOR_PORT_ADDRESS}

    # VectorTableOffset should not yet have been updated
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_INITIAL_ADDRESS}

    Execute Command                 cpu Reset
    Wait For Log Entry              cpu: VectorTableOffset set to ${VTOR_PORT_ADDRESS}

    # VectorTableOffset should have been updated out of reset
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_PORT_ADDRESS}

    # The non-secure offset should remain unchanged
    ${vtor_non_secure}=             Execute Command  cpu VectorTableOffsetNonSecure
    Should Be Equal As Integers     ${vtor_non_secure}  ${VTOR_INITIAL_ADDRESS}

SystemC Should Not Set Vector Table Offset Register Using Secure Port With TrustZone Disabled
    Create Machine With Trust Zone Disabled

    # Fast Model documentation states that `initsvtor` should be ignored when ARMv8-M Security Extensions are not included
    Trigger SystemC Signal ${SIGNAL_INIT_S_VTOR}
    Wait For Log Entry              signals: The Security Extension is not enabled. Ignoring Secure Vector table offset signal

    # VectorTableOffset should not have been updated
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_INITIAL_ADDRESS}

    Execute Command                 cpu Reset
    Wait For Log Entry              cpu: VectorTableOffset set to ${VTOR_INITIAL_ADDRESS}

    # The offset should have stayed unchanged from the previous write
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_INITIAL_ADDRESS}

SystemC Should Set Vector Table Offset Register Using Non Secure Port With TrustZone Disabled
    Create Machine With Trust Zone Disabled

    # When TrustZone is disabled, the Secure/Non-Secure distinction does not exist,
    # so VectorTableOffsetNonSecure has no meaning. Renode uses VectorTableOffset
    # to represent the single unified vector table offset in this case.
    Trigger SystemC Signal ${SIGNAL_INIT_NS_VTOR}
    Wait For Log Entry              signals: SystemC Non Secure Vector Table Offset: ${VTOR_NON_SECURE_PORT_ADDRESS}

    # VectorTableOffset should not yet have been updated
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_INITIAL_ADDRESS}

    Execute Command                 cpu Reset
    Wait For Log Entry              cpu: VectorTableOffset set to ${VTOR_NON_SECURE_PORT_ADDRESS}

    # VectorTableOffset should have been updated out of reset
    ${vtor}=                        Execute Command  cpu VectorTableOffset
    Should Be Equal As Integers     ${vtor}  ${VTOR_NON_SECURE_PORT_ADDRESS}
