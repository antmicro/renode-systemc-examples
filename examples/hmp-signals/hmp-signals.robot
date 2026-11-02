*** Variables ***
${SYSTEMC_BASE}                     0x60000000
${SYSTEMC_CODE}                     0x60001000
${RESET_OFFSET}                     0x100
${NMI_OFFSET}                       0x104
${BOOT_OFFSET}                      0x108
${CLOCK_OFFSET}                     0x110
${BUS_TRIGGER_OFFSET}               0x114
${CPU_WAIT_OFFSET}                  0x118
${RESET_REQUEST_OFFSET}             0x11c

*** Keywords ***
Create Machine
    Execute Command                 include @${CURDIR}/renode/hmp-signals.resc
    Create Log Tester               1  defaultPauseEmulation=true
    Execute Command                 logLevel -1 signals

Read Integer
    [Arguments]                     ${command}
    ${raw}=                         Execute Command  ${command}
    ${value}=                       Evaluate  int($raw.strip(), 0)
    RETURN                          ${value}

Integer Should Be
    [Arguments]                     ${command}  ${expected}
    ${value}=                       Read Integer  ${command}
    Should Be Equal As Integers     ${value}  ${expected}

State Should Be
    [Arguments]                     ${command}  ${expected}
    ${value}=                       Execute Command  ${command}
    Should Be Equal                 ${value.strip()}  ${expected}

Write Control
    [Arguments]                     ${cpu}  ${offset}  ${value}
    Execute Command                 sysbus WriteDoubleWord ${{${SYSTEMC_BASE} + ${offset}}} ${value} context=${cpu}
    # Let deferred clock/reset changes take effect before checking state.
    Execute Command                 emulation RunFor "0.0003"

Reset Core
    [Arguments]                     ${cpu}
    Write Control                   ${cpu}  ${RESET_OFFSET}  1
    State Should Be                 ${cpu} IsHalted  True
    Write Control                   ${cpu}  ${RESET_OFFSET}  0
    State Should Be                 ${cpu} IsHalted  False

Start Both Cores
    Execute Command                 riscv IsHalted false
    Execute Command                 arm IsHalted false
    Execute Command                 emulation RunFor "0.001"

*** Test Cases ***
Both Architectures Should Execute From Their Own SystemC Memory
    Create Machine
    Execute Command                 riscv AssembleBlock ${SYSTEMC_CODE} "addi x5, zero, 42; j ."
    Execute Command                 arm AssembleBlock ${SYSTEMC_CODE} "movs r5, #43; b ."
    Execute Command                 riscv PC ${SYSTEMC_CODE}
    Execute Command                 arm PC ${SYSTEMC_CODE}
    Start Both Cores

    Integer Should Be               riscv GetRegister 5  42
    Integer Should Be               arm GetRegister 5  43
    Integer Should Be               riscv PC  ${{${SYSTEMC_CODE} + 4}}
    Integer Should Be               arm PC  ${{${SYSTEMC_CODE} + 2}}

Clock Changes Should Affect Only The Selected Bundle
    Create Machine
    Integer Should Be               clint Frequency  100000000
    Integer Should Be               nvic Frequency  50000000
    Write Control                   riscv  ${CLOCK_OFFSET}  0
    State Should Be                 riscv Clocked  False
    State Should Be                 arm Clocked  True
    State Should Be                 nvic Clocked  True

    Write Control                   riscv  ${CLOCK_OFFSET}  1000
    Integer Should Be               clint Frequency  1000000
    Integer Should Be               nvic Frequency  50000000
    Write Control                   arm  ${CLOCK_OFFSET}  0
    State Should Be                 riscv Clocked  True
    State Should Be                 arm Clocked  False
    State Should Be                 nvic Clocked  False
    State Should Be                 dwt Clocked  False

    Write Control                   arm  ${CLOCK_OFFSET}  500
    State Should Be                 arm Clocked  True
    Integer Should Be               nvic Frequency  2000000
    Integer Should Be               dwt Frequency  2000000
    Integer Should Be               clint Frequency  1000000

Same GPIO Number Should Reach The Selected Interrupt Controller
    Create Machine
    Execute Command                 plic WriteDoubleWord 0x2000 0x2
    Execute Command                 sysbus WriteByte ${{${SYSTEMC_BASE} + 1}} 1 context=riscv
    Execute Command                 emulation RunToNearestSyncPoint
    Integer Should Be               plic ReadDoubleWord 0x200004  1
    Integer Should Be               nvic ReadDoubleWord 0x200  0

    Execute Command                 sysbus WriteByte ${{${SYSTEMC_BASE} + 1}} 0 context=riscv
    Execute Command                 plic WriteDoubleWord 0x200004 1
    Execute Command                 sysbus WriteByte ${{${SYSTEMC_BASE} + 1}} 1 context=arm
    Execute Command                 emulation RunToNearestSyncPoint
    Integer Should Be               nvic ReadDoubleWord 0x200  2
    Integer Should Be               plic ReadDoubleWord 0x200004  0

Reset Should Leave The Other Architecture Running
    Create Machine
    Execute Command                 riscv AssembleBlock 0x80000000 "loop: addi x4, x4, 1; j loop"
    Execute Command                 arm AssembleBlock 0x20000100 "loop: adds r4, #1; b loop"
    Start Both Cores
    Execute Command                 plic WriteDoubleWord 0x2000 0x2
    Execute Command                 dwt WriteDoubleWord 0 1
    Write Control                   riscv  ${RESET_OFFSET}  1
    ${before}=                      Read Integer  arm ExecutedInstructions
    Execute Command                 emulation RunFor "0.001"
    ${after}=                       Read Integer  arm ExecutedInstructions
    Should Be True                  ${after} > ${before}
    State Should Be                 riscv IsHalted  True
    Write Control                   riscv  ${RESET_OFFSET}  0
    Integer Should Be               plic ReadDoubleWord 0x2000  0
    Integer Should Be               dwt ReadDoubleWord 0  1

    Execute Command                 plic WriteDoubleWord 0x2000 0x2
    Write Control                   arm  ${RESET_OFFSET}  1
    ${before}=                      Read Integer  riscv ExecutedInstructions
    Execute Command                 emulation RunFor "0.001"
    ${after}=                       Read Integer  riscv ExecutedInstructions
    Should Be True                  ${after} > ${before}
    State Should Be                 arm IsHalted  True
    Write Control                   arm  ${RESET_OFFSET}  0
    Integer Should Be               dwt ReadDoubleWord 0  0
    Integer Should Be               plic ReadDoubleWord 0x2000  2
    State Should Be                 arm IsHalted  False

Boot Address And Vector Table Should Be Separate
    Create Machine
    Execute Command                 sysbus WriteDoubleWord 0x20000400 0x20010000
    Execute Command                 sysbus WriteDoubleWord 0x20000404 0x20000501
    Execute Command                 riscv AssembleBlock 0x80000100 "j ."
    Execute Command                 arm AssembleBlock 0x20000500 "b ."
    Write Control                   riscv  ${BOOT_OFFSET}  0x80000100
    Write Control                   arm  ${BOOT_OFFSET}  0x20000400
    Integer Should Be               riscv ResetVector  0x80000100
    Integer Should Be               arm InitVectorTableOffset  0x20000400
    Reset Core                      riscv
    Reset Core                      arm
    Execute Command                 emulation RunFor "0.001"
    Integer Should Be               riscv PC  0x80000100
    Integer Should Be               arm PC  0x20000500

NMI Should Reach Only The Selected Architecture
    Create Machine
    Execute Command                 sysbus WriteDoubleWord 0x20000008 0x20000201
    Execute Command                 arm AssembleBlock 0x20000200 "b ."
    Start Both Cores
    Write Control                   riscv  ${NMI_OFFSET}  1
    Execute Command                 emulation RunFor "0.001"
    Integer Should Be               riscv PC  0x80000200
    Integer Should Be               arm PC  0x20000100
    Write Control                   arm  ${NMI_OFFSET}  1
    Execute Command                 emulation RunFor "0.001"
    Integer Should Be               arm PC  0x20000200
    Integer Should Be               riscv PC  0x80000200

ARM Reset Request Should Return Through Its Bridge
    Create Machine
    # AIRCR.SYSRESETREQ drives an outgoing GPIO on the ARM bridge (ID 1).
    Execute Command                 nvic WriteDoubleWord 0xd0c 0x05fa0004
    Integer Should Be               sysbus ReadByte ${{${SYSTEMC_BASE} + ${RESET_REQUEST_OFFSET}}} context=arm  1
    Integer Should Be               sysbus ReadByte ${{${SYSTEMC_BASE} + ${RESET_REQUEST_OFFSET}}} context=riscv  0

Both Bridges Should Perform System Bus Transactions
    Create Machine
    Write Control                   riscv  ${BUS_TRIGGER_OFFSET}  1
    Write Control                   arm  ${BUS_TRIGGER_OFFSET}  1
    Integer Should Be               sysbus ReadDoubleWord 0x80001000  0x5a5aa5a5
    Integer Should Be               sysbus ReadDoubleWord 0x20001000  0x5a5aa5a6

Duplicate CPU IDs Should Be Rejected
    Execute Command                 mach create
    ${platform}=                    Get File  ${CURDIR}/renode/hmp-signals.repl
    ${platform}=                    Replace String  ${platform}  cpuId: 1  cpuId: 0
    Run Keyword And Expect Error    *Multiple CPU signal bundles use multiprocessing ID 0*
    ...                             Execute Command  machine LoadPlatformDescriptionFromString """${platform}"""
