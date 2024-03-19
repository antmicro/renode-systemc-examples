### Transaction delay on STM32 Mini F401

This example is very similar to `timesync`, but additionally the
read transaction from the SystemC peripheral is delayed through the `delay`
parameter in the TLM `b_transport` call. The result is that the time value read
from `timekeeper` arrives with one second delay, which can be confirmed by
looking at the UART output.

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
