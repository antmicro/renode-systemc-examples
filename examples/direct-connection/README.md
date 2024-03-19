### Direct connection

This example illustrates a case where there are multiple SystemC peripherals.
It also illustrates the use of the `direct connection` functionality, allowing
peripherals to communicate using memory-mapped transfers not going through the
system bus. This, for example, allows them to communicate using a different
address space than the system bus address space.

The peripherals hold a single value, which can be changed by writing to the
peripheral. When read from, the peripheral will read the value not from itself,
but from the other peripheral, using the direct connection. This is transparent from
the point of view of the Renode system bus. The example works by writing values
to the two peripherals, and illustrating that reading from one of them returns
the value saved in the other.


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
