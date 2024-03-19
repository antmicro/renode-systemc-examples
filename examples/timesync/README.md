### Time synchronization on STM32 Mini F401

This example illustrates virtual time synchronization between Renode and
SystemC. It is also simulated within a more complex platform (the [STM32 Mini
F401](https://renodepedia.renode.io/boards/stm32f401_mini/?view=software&demo=hello_world),
extended with the SystemC peripheral). The SystemC part is a simple timer that
returns SystemC virtual time in seconds when read from. The Renode simulation
reads it and outputs the obtained time to the UART. It can then be compared with
Renode virtual time to confirm that they are synchronized.

``` raw
                 Renode process                          SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
       ┌─────────────────┐        TCP     ┌─────────────┐  ┌────────────┐
│      │SystemCPeripheral├──────┼──────┼──┤renode_bridge├──┤ timekeeper │ │
       └───────┬─────────┘                └─────────────┘  └────────────┘
│              │                │      └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
  ┌────────────┴───────────────┐ 
│ │      STM32 Mini F401       ││
  └────────────────────────────┘ 
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```
