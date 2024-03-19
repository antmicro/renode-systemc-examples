### Non-blocking TLM binding

This example is almost identical to `direct-connection`. The most important
difference is that the peripherals use a non-blocking TLM interface. This is just
a matter of syntax, as the integration still handles the transactions in a
single phase.


``` raw
                 Renode process                          SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
  ┌──────┐   ┌─────────────────┐        TCP     ┌─────────────┐  ┌────────────┐
│ │      ├───┤SystemCPeripheral├──────┼──────┼──┤renode_bridge├──┤ peripheral │ │
  │      │   └───────┬─────────┘                └─────────────┘  └────────────┘
│ │      │           │                │      └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
  │      │           │ direct          
│ │sysbus│           │ connection     │
  │      │           │                                         SystemC process    
│ │      │           │                │      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐ 
  │      │   ┌───────┴─────────┐        TCP     ┌─────────────┐  ┌────────────┐
│ │      ├───┤SystemCPeripheral├──────┼──────┼──┤renode_bridge├──┤ peripheral │ │
  └──────┘   └─────────────────┘                └─────────────┘  └────────────┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘      └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
```
