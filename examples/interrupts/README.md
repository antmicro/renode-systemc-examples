### Interrupts and GPIO

This example shows how to model interrupt requests sent through the Renode -
SystemC interface.

The SystemC part contains two `interrupter` modules, each sending IRQs to Renode.
"Interrupters" are connected by a simple bus module, routing the requests
to one or the other based on the address.

Interrupter modules connect their interrupt lines directly to `renode_bridge`
through SystemC ports bound to `sc_signal<bool>` channels.

The two interrupter modules raise interrupts in fixed periods. The interrupt
signals are cleared when any value is written to the interrupters. The Renode side
of the simulation sets up two interrupt handlers - one for each interrupter.
When the Renode-emulated CPU receives an interrupt, an output message is
written to the UART and the interrupt signal is cleared by writing to the
appropriate interrupter memory address.

``` raw
                 Renode process                          SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
       ┌─────────────────┐        TCP     ┌─────────────┐  ┌────────────┐  ┌────────────┐
│      │SystemCPeripheral├──────┼──────┼──┤renode_bridge│  │interrupter0│  │interrupter1│ │
       └───────┬─────────┘                └──────┬──────┘  └─────┬──────┘  └──────┬─────┘  
│              │                │      │  ┌──────┴───────────────┴────────────────┴─────┐ │
  ┌────────────┴───────────────┐          │                    bus                      │  
│ │      STM32 Mini F401       ││      │  └─────────────────────────────────────────────┘ │
  └────────────────────────────┘       └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```
