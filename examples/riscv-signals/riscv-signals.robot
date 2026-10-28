*** Variables ***
${REPL_PATH}                        ${CURDIR}/renode/riscv-signals.repl
${SYSTEMC_BINARY}                   ${CURDIR}/bin/riscv-signals

${BOOT_ADDRESS}                     0x80000000
${ALTERNATE_BOOT_ADDRESS}           0x80000100
${NMI_VECTOR}                       0x80000200

${RESET_OFFSET}                     0x100
${NMI_OFFSET}                       0x104
${BOOT_ADDRESS_OFFSET}              0x108
${CLOCK_PERIOD_OFFSET}              0x110
${SYSTEM_BUS_TRIGGER_OFFSET}        0x114

${MEMORY_OFFSET}                    0x1000
${SYSTEMC_CODE_ADDRESS}             0x90001000
${SYSTEM_BUS_TEST_ADDRESS}          0x80001000
${SYSTEM_BUS_TEST_VALUE}            0x5a5aa5a5

*** Keywords ***
Create Machine
    [Arguments]                     ${direct_interrupts}=${False}  ${interrupt_count}=16  ${reset_active}=High
    Execute Command                 mach create "riscv-signals"
    ${platform}=                    Get File  ${REPL_PATH}
    IF  ${direct_interrupts}
        ${platform}=                Replace String  ${platform}  interruptController: plic  ${EMPTY}
    END
    IF  $interrupt_count is None
        ${platform}=                Replace String  ${platform}  numberOfInterrupts: 16  ${EMPTY}
    ELSE
        ${platform}=                Replace String  ${platform}  numberOfInterrupts: 16  numberOfInterrupts: ${interrupt_count}
    END
    ${platform}=                    Catenate  SEPARATOR=${\n}  ${platform}  riscvBundle: { resetInActive: SystemC.SignalActiveWhen.${reset_active} }
    Execute Command                 machine LoadPlatformDescriptionFromString """${platform}"""
    # Reset releases the CPU, so provide a valid reset handler for every test.
    Execute Command                 cpu AssembleBlock ${BOOT_ADDRESS} "j ."
    Execute Command                 sysbus LogPeripheralAccess signals true
    Execute Command                 sysbus.signals SystemCExecutablePath @${SYSTEMC_BINARY}
    Create Log Tester               1  defaultPauseEmulation=true
    Execute Command                 logLevel -1 signals

Read Peripheral Double Word
    [Arguments]                     ${peripheral}  ${offset}
    ${raw}=                         Execute Command  ${peripheral} ReadDoubleWord ${offset}
    ${value}=                       Evaluate  int($raw.strip(), 0)
    RETURN                          ${value}

Read CPU Register
    [Arguments]                     ${register}
    ${raw}=                         Execute Command  cpu ${register}
    ${value}=                       Evaluate  int($raw.strip(), 0)
    RETURN                          ${value}

Write Signal
    [Arguments]                     ${offset}  ${value}
    Execute Command                 signals WriteByte ${offset} ${value}

Trigger Reset
    Write Signal                    ${RESET_OFFSET}  1
    Wait For Log Entry              signals: CPU halted after reset signal assertion  level=Debug

Release Reset
    Write Signal                    ${RESET_OFFSET}  0
    Wait For Log Entry              signals: CPU and peripherals were reset after signal deassertion  level=Debug

Prepare CPU
    Execute Command                 cpu AssembleBlock ${NMI_VECTOR} "j ."
    Execute Command                 cpu PC ${BOOT_ADDRESS}
    Execute Command                 cpu IsHalted false

Instantaneous CPU Clocked State Should Be
    [Arguments]                     ${expected}
    ${clocked}=                     Execute Command  cpu Clocked
    Should Be Equal                 ${clocked.strip()}  ${expected}

CPU Should Be Clocked
    [Arguments]                     ${expected}
    Wait Until Keyword Succeeds     20x  0.1s  Instantaneous CPU Clocked State Should Be  ${expected}

System Bus Should Contain Test Value
    ${value}=                       Read Peripheral Double Word  sysbus  ${SYSTEM_BUS_TEST_ADDRESS}
    Should Be Equal As Integers     ${value}  ${SYSTEM_BUS_TEST_VALUE}

PLIC Should Claim Source
    [Arguments]                     ${expected}
    ${source}=                      Read Peripheral Double Word  plic  0x200004
    Should Be Equal As Integers     ${source}  ${expected}

CPU Interrupt Pending State Should Be
    [Arguments]                     ${interrupt}  ${expected}
    ${pending}=                     Read CPU Register  MIP
    ${is_pending}=                  Evaluate  bool($pending & (1 << ${interrupt}))
    Should Be Equal                 ${is_pending}  ${expected}

*** Test Cases ***
Should Forward Renode Accesses To SystemC Memory
    Create Machine

    Execute Command                 signals WriteDoubleWord ${MEMORY_OFFSET} 0xdeadbeef
    ${memory_value}=                Read Peripheral Double Word  signals  ${MEMORY_OFFSET}
    Should Be Equal As Integers     ${memory_value}  0xdeadbeef

CPU Should Execute Code From SystemC Memory
    Create Machine
    Execute Command                 cpu AssembleBlock ${SYSTEMC_CODE_ADDRESS} "addi x5, zero, 42; j ."
    Execute Command                 cpu PC ${SYSTEMC_CODE_ADDRESS}
    Execute Command                 cpu IsHalted false
    Execute Command                 emulation RunFor "0.001"

    ${value}=                       Read CPU Register  GetRegister 5
    Should Be Equal As Integers     ${value}  42
    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${{${SYSTEMC_CODE_ADDRESS} + 4}}

Should Allow SystemC To Initiate System Bus Transactions
    Create Machine

    Execute Command                 signals WriteDoubleWord ${SYSTEM_BUS_TRIGGER_OFFSET} 1

    Wait Until Keyword Succeeds     20x  0.1s  System Bus Should Contain Test Value

Reset Input Should Reset CPU And Interrupt Controller
    Create Machine

    Execute Command                 plic WriteDoubleWord 0x2000 0x2  # Enable PLIC source 1
    Execute Command                 cpu PC 0xdeadbeef

    ${plic_enabled}=                Read Peripheral Double Word  plic  0x2000
    Should Be Equal As Integers     ${plic_enabled}  0x2

    Trigger Reset
    ${is_halted}=                   Execute Command  cpu IsHalted
    Should Be Equal                 ${is_halted.strip()}  True

    Release Reset

    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${BOOT_ADDRESS}

    ${plic_enabled}=                Read Peripheral Double Word  plic  0x2000
    Should Be Equal As Integers     ${plic_enabled}  0

CPU Should Resume Execution After Reset
    Create Machine
    Prepare CPU
    Execute Command                 emulation RunFor "0.001"

    Trigger Reset
    ${before}=                      Read CPU Register  ExecutedInstructions
    Execute Command                 emulation RunFor "0.001"
    ${after}=                       Read CPU Register  ExecutedInstructions
    Should Be Equal As Integers     ${before}  ${after}

    Release Reset
    ${halted}=                      Execute Command  cpu IsHalted
    Should Be Equal                 ${halted.strip()}  False
    ${before}=                      Read CPU Register  ExecutedInstructions
    Execute Command                 emulation RunFor "0.001"
    ${after}=                       Read CPU Register  ExecutedInstructions
    Should Be True                  ${after} > ${before}

Clock Frequency Should Survive Reset
    Create Machine
    Execute Command                 signals WriteDoubleWord ${CLOCK_PERIOD_OFFSET} 1000
    Execute Command                 emulation RunToNearestSyncPoint
    Trigger Reset
    Release Reset
    Execute Command                 cpu IsHalted true

    ${before}=                      Read Peripheral Double Word  clint  0xbff8
    Execute Command                 emulation RunFor "0.001"
    ${after}=                       Read Peripheral Double Word  clint  0xbff8
    Should Be Equal As Integers     ${{${after} - ${before}}}  1000

NMI Input Should Redirect CPU To NMI Vector
    Create Machine
    Prepare CPU

    Write Signal                    ${NMI_OFFSET}  1
    Execute Command                 emulation RunFor "0.001"

    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${NMI_VECTOR}

    ${mcause}=                      Read CPU Register  MCAUSE
    Should Be Equal As Integers     ${mcause}  0

    Write Signal                    ${NMI_OFFSET}  0

Boot Address Input Should Change Reset Vector
    Create Machine

    Execute Command                 cpu AssembleBlock ${ALTERNATE_BOOT_ADDRESS} "j ."
    Execute Command                 signals WriteDoubleWord ${BOOT_ADDRESS_OFFSET} ${ALTERNATE_BOOT_ADDRESS}

    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${ALTERNATE_BOOT_ADDRESS}

    Trigger Reset
    Release Reset

    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${ALTERNATE_BOOT_ADDRESS}

Boot Address Change After CPU Reset Should Preserve Current PC
    Create Machine
    Prepare CPU
    Execute Command                 cpu AssembleBlock ${ALTERNATE_BOOT_ADDRESS} "j ."
    Execute Command                 emulation RunFor "0.001"
    Execute Command                 cpu Reset
    Execute Command                 cpu IsHalted true

    Execute Command                 signals WriteDoubleWord ${BOOT_ADDRESS_OFFSET} ${ALTERNATE_BOOT_ADDRESS}
    Execute Command                 emulation RunToNearestSyncPoint
    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${BOOT_ADDRESS}

    Trigger Reset
    Release Reset
    ${pc}=                          Read CPU Register  PC
    Should Be Equal As Integers     ${pc}  ${ALTERNATE_BOOT_ADDRESS}

Clock Period Input Should Control CPU Clocking
    Create Machine

    CPU Should Be Clocked           True

    Execute Command                 signals WriteDoubleWord ${CLOCK_PERIOD_OFFSET} 0
    Execute Command                 emulation RunToNearestSyncPoint
    CPU Should Be Clocked           False

    Execute Command                 signals WriteDoubleWord ${CLOCK_PERIOD_OFFSET} 1000
    Execute Command                 emulation RunToNearestSyncPoint
    CPU Should Be Clocked           True

    ${frequency}=                   Execute Command  clint Frequency
    ${frequency}=                   Evaluate  int($frequency.strip(), 0)
    Should Be Equal As Integers     ${frequency}  1000000

Interrupt Input Should Reach Interrupt Controller
    Create Machine

    Execute Command                 plic WriteDoubleWord 0x2000 0x2  # Enable PLIC source 1
    Write Signal                    1  1

    Wait Until Keyword Succeeds     20x  0.1s  PLIC Should Claim Source  1

    Write Signal                    1  0

Highest Interrupt Input Should Reach Interrupt Controller
    Create Machine
    Execute Command                 plic WriteDoubleWord 0x2000 0x8000
    Write Signal                    15  1
    Wait Until Keyword Succeeds     20x  0.1s  PLIC Should Claim Source  15

Asserted Interrupt Input Should Survive Reset
    Create Machine
    Write Signal                    1  1
    Trigger Reset
    Release Reset

    Execute Command                 plic WriteDoubleWord 0x2000 0x2
    PLIC Should Claim Source        1

Direct Interrupt Input Should Reach CPU
    Create Machine                  direct_interrupts=${True}  interrupt_count=${None}
    Write Signal                    11  1
    Wait Until Keyword Succeeds     20x  0.1s  CPU Interrupt Pending State Should Be  11  ${True}
    Write Signal                    11  0
    Wait Until Keyword Succeeds     20x  0.1s  CPU Interrupt Pending State Should Be  11  ${False}

Direct Interrupt Inputs Must Fit CPU Width
    Run Keyword And Expect Error    *Invalid number of interrupt inputs: 33*
    ...                             Create Machine  direct_interrupts=${True}  interrupt_count=33

Active Low Reset Should Hold And Release CPU
    Create Machine                  reset_active=Low
    Execute Command                 emulation RunToNearestSyncPoint
    ${halted}=                      Execute Command  cpu IsHalted
    Should Be Equal                 ${halted.strip()}  True

    Write Signal                    ${RESET_OFFSET}  1
    Wait For Log Entry              signals: CPU and peripherals were reset after signal deassertion  level=Debug
    ${halted}=                      Execute Command  cpu IsHalted
    Should Be Equal                 ${halted.strip()}  False
