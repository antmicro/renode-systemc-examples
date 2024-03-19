### Simple DMA

This example shows a rudimentary DMA controller (DMAC) peripheral implemented in
SystemC and connected to Renode. It illustrates SystemC-to-Renode bus
communication and handling of Renode-initiated GPIO signals.

The Renode script sets up the DMAC by setting the appropriate source, destination
and data length registers. It then starts the memory transfer by writing to the
DMAC control register. Next, DMAC is informed that the bus is free by Renode
raising a "bus free" signal on one of the GPIO pins on the SystemC
peripheral. The DMAC then performs a memory-to-memory transfer according to the
setup.


``` raw
                 Renode process                         SystemC process   
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐     ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
       ┌─────────────────┐        TCP    ┌─────────────┐  ┌────────────┐  
│      │SystemCPeripheral├──────┼─────┼──┤renode_bridge├──┤    DMAC    │ │
       └───────┬─────────┘               └─────────────┘  └────────────┘  
│              │                │     └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
  ┌────────────┴───────────────┐
│ │      STM32 Mini F401       ││
  └────────────────────────────┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```
