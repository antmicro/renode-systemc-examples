*** Variables ***
${REPL_PATH}                        ${CURDIR}/renode/multi-core.repl
${SYSTEMC_BINARY}                   ${CURDIR}/bin/multi-core

${QUANTUM}                          0.00001
${RUN_SLICE}                        0.0002

# SystemC shared memory spans 0x20000000-0x2001ffff.
${M55_PC}                           0x20000000
${M0_PC}                            0x20001000

# Each core owns a block in the shared memory with the following layout:
${MAILBOX}                          0x0  # value posted for the other core
${ECHO}                             0x4  # what this core read from the other core's mailbox
${DONE}                             0x8  # set to 1 when the program finishes
${INCREMENTS}                       0xc  # how many times this core bumped the counter
${BLOCK_M55}                        0x20003000
${BLOCK_M0}                         0x20003010
${COUNTER}                          0x20003020

${MAGIC_M55}                        0xCAFE0000
${MAGIC_M0}                         0xBEEF0001
${PING_PONG_LIMIT}                  32

# r0: own block, r1: other core's block, r2: own magic, r3: other core's magic.
${MAILBOX_ASSEMBLY}                 SEPARATOR=\n
...                                 str r2, [r0, #${MAILBOX}]
...                                 wait:
...                                 ldr r4, [r1, #${MAILBOX}]
...                                 cmp r4, r3
...                                 bne wait
...                                 str r4, [r0, #${ECHO}]
...                                 movs r4, #1
...                                 str r4, [r0, #${DONE}]
...                                 b .

# r0: own block, r1: counter address, r2: parity of the counter this core may increment.
# r5 counts how many increments this core performed. It is stored to INCREMENTS at the end.
${PING_PONG_ASSEMBLY}               SEPARATOR=\n
...                                 movs r4, #${PING_PONG_LIMIT}
...                                 movs r5, #0
...                                 loop:
...                                 ldr r3, [r1]
...                                 cmp r3, r4
...                                 bge done
...                                 movs r6, #1
...                                 ands r6, r3
...                                 cmp r6, r2
...                                 bne loop
...                                 adds r3, #1
...                                 str r3, [r1]
...                                 adds r5, #1
...                                 b loop
...                                 done:
...                                 str r5, [r0, #${INCREMENTS}]
...                                 movs r3, #1
...                                 str r3, [r0, #${DONE}]
...                                 b .

*** Keywords ***
Create Machine
    Execute Command                 mach create "multi-core"
    Execute Command                 machine LoadPlatformDescription @${REPL_PATH}
    Execute Command                 emulation SetGlobalQuantum "${QUANTUM}"
    Execute Command                 sysbus.cortexMProxy SystemCExecutablePath @${SYSTEMC_BINARY}
    Execute Command                 cortexMProxy DisableDebugAccess false
    Execute Command                 emulation SetGlobalSerialExecution true

Start Core
    [Arguments]                     ${cpu}  ${pc}  ${assembly}  &{registers}
    Execute Command                 ${cpu} AssembleBlock ${pc} """${assembly}"""
    Execute Command                 ${cpu} PC ${pc}
    FOR  ${register}  ${value}  IN  &{registers}
        Execute Command                 ${cpu} SetRegister "${register}" ${value}
    END
    Execute Command                 ${cpu} IsHalted false

Read Shared Word
    [Arguments]                     ${address}  ${context}
    ${value}=                       Execute Command  sysbus ReadDoubleWord ${address} context=${context}
    ${value}=                       Convert To Integer  ${value.strip()}
    RETURN                          ${value}

Write Shared Word
    [Arguments]                     ${address}  ${value}  ${context}
    Execute Command                 sysbus WriteDoubleWord ${address} ${value} context=${context}

Shared Word Should Be
    [Arguments]                     ${address}  ${expected}  ${message}
    ${m55_view}=                    Read Shared Word  ${address}  context=m55
    ${m0_view}=                     Read Shared Word  ${address}  context=m0
    Should Be Equal As Integers     ${m55_view}  ${expected}  ${message} (seen by m55)
    Should Be Equal As Integers     ${m0_view}  ${expected}  ${message} (seen by m0)

Run Until Both Cores Are Done
    Execute Command                 emulation RunFor "${RUN_SLICE}"
    ${done_m55}=                    Read Shared Word  ${{ ${BLOCK_M55} + ${DONE} }}  context=m55
    ${done_m0}=                     Read Shared Word  ${{ ${BLOCK_M0} + ${DONE} }}  context=m0

    Should Be Equal As Integers     ${done_m55}  1  m55 did not finish
    Should Be Equal As Integers     ${done_m0}  1  m0 did not finish

*** Test Cases ***
Both Cores Should See The Same Shared Memory
    Create Machine

    Write Shared Word               ${BLOCK_M55}  0x11223344  context=m55
    ${from_m0}=                     Read Shared Word  ${BLOCK_M55}  context=m0
    Should Be Equal As Integers     ${from_m0}  0x11223344  write via m55 not visible via m0

    Write Shared Word               ${BLOCK_M55}  0x55667788  context=m0
    ${from_m55}=                    Read Shared Word  ${BLOCK_M55}  context=m55
    Should Be Equal As Integers     ${from_m55}  0x55667788  write via m0 not visible via m55

Cores Should Exchange Data Through Shared Memory
    Create Machine
    Start Core                      m55  ${M55_PC}  ${MAILBOX_ASSEMBLY}
    ...                             r0=${BLOCK_M55}  r1=${BLOCK_M0}  r2=${MAGIC_M55}  r3=${MAGIC_M0}
    Start Core                      m0  ${M0_PC}  ${MAILBOX_ASSEMBLY}
    ...                             r0=${BLOCK_M0}  r1=${BLOCK_M55}  r2=${MAGIC_M0}  r3=${MAGIC_M55}

    Run Until Both Cores Are Done

    Shared Word Should Be           ${{ ${BLOCK_M55} + ${ECHO} }}  ${MAGIC_M0}  m55 did not read m0's magic value
    Shared Word Should Be           ${{ ${BLOCK_M0} + ${ECHO} }}  ${MAGIC_M55}  m0 did not read m55's magic value

Cores Should Take Turns Incrementing A Shared Counter
    Create Machine
    Start Core                      m55  ${M55_PC}  ${PING_PONG_ASSEMBLY}
    ...                             r0=${BLOCK_M55}  r1=${COUNTER}  r2=0
    Start Core                      m0  ${M0_PC}  ${PING_PONG_ASSEMBLY}
    ...                             r0=${BLOCK_M0}  r1=${COUNTER}  r2=1

    Run Until Both Cores Are Done

    Shared Word Should Be           ${COUNTER}  ${PING_PONG_LIMIT}  counter did not reach the limit
    Shared Word Should Be           ${{ ${BLOCK_M55} + ${INCREMENTS} }}  ${{ ${PING_PONG_LIMIT} // 2 }}  m55 did not do exactly half of the increments
    Shared Word Should Be           ${{ ${BLOCK_M0} + ${INCREMENTS} }}  ${{ ${PING_PONG_LIMIT} // 2 }}  m0 did not do exactly half of the increments
