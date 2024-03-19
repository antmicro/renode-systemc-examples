*** Variables ***
${SCRIPT_PATH}                      ${CURDIR}/renode/dma.resc

*** Keywords ***
Memory Should Be Equal To
    [Arguments]                     ${address}  ${value}
    ${res}=                         Execute Command  sysbus ReadByte ${address}
    Should Be Equal As Numbers      ${res}  ${value}

*** Test Cases ***
Should Perform Memory-To-Memory Transfer
    Execute Script                  ${SCRIPT_PATH}
    Start Emulation

    # Initialize memory
    Execute Command                 memory WriteString 0 "Hello"

    # Make sure memory destination is initialized to 0
    Memory Should Be Equal To       0x20000010  0
    Memory Should Be Equal To       0x20000011  0
    Memory Should Be Equal To       0x20000012  0
    Memory Should Be Equal To       0x20000013  0
    Memory Should Be Equal To       0x20000014  0

    # Set source address register in dma_systemc.
    Execute Command                 sysbus WriteDoubleWord 0x9000004 0x20000000

    # Set destination address register in dma_systemc.
    Execute Command                 sysbus WriteDoubleWord 0x9000008 0x20000010

    # Set data length register (in bytes) in dma_systemc.
    Execute Command                 sysbus WriteDoubleWord 0x900000C 5

    # Start memory-to-memory transfer in dma_systemc
    Execute Command                 sysbus WriteDoubleWord 0x9000010 0x1

    # Signal "bus free" to notify DMAC it can use the bus now.
    Execute Command                 sysbus.dma_systemc OnGPIO 2 True

    # Verify the memory was copied correctly.
    # "H"
    Memory Should Be Equal To       0x20000010  0x48
    # "e"
    Memory Should Be Equal To       0x20000011  0x65
    # "l"
    Memory Should Be Equal To       0x20000012  0x6C
    # "l"
    Memory Should Be Equal To       0x20000013  0x6C
    # "o"
    Memory Should Be Equal To       0x20000014  0x6F
