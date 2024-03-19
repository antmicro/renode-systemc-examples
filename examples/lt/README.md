# Loosely-timed SystemC example (`lt`)

This example illustrates the integration of Renode with a non-trivial SystemC
model. It is an adaptation of the "loosely-timed" example, which is part of the
official SystemC-2.3.4 distribution.

The SystemC model consists of two `initiator` modules that generate traffic, two
"memory" modules that implement simple memories and a bus module that connects
them together.

The above has been extended so that the Renode simulation serves as a third
"initiator" module, reading and writing to the memories modelled in the
SystemC process via `sysbus read/write ...` Renode commands.

``` raw
                 Renode process                                            SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
       ┌─────────────────┐        TCP     ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  
│      │SystemCPeripheral├──────┼──────┼──┤renode_bridge│  │lt_initiator0│  │lt_initiator1│ │
       └───────┬─────────┘                └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  
│              │                │      │         │                │                │        │
  ┌────────────┴───────────────┐          ┌──────┴────────────────┴────────────────┴──────┐  
│ │          sysbus            ││      │  │                  SimpleBusLT                  │ │
  └────────────────────────────┘          └─────────┬───────────────────────────┬─────────┘  
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘      │            │                           │           │
                                                ┌───┴───┐                   ┌───┴───┐        
                                       │        │memory0│                   │memory1│       │
                                                └───────┘                   └───────┘        
                                       └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
```
