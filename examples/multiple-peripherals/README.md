# Multiple peripherals

## System structure

Starting with a simulation fully implemented in SystemC, with the
following structure:

``` raw
fifo_in
┌─────────►┌──────────────┐  register_if
│   mem_if │ Peripheral A │◄─────────────────┐
│   ┌─────►└─┬───┬────────┘                  │
│   │    dma │   │ fifo_out                  │
│   │        │   │                           │
│   │        │   │                           │
│   │ mem_if ▼   ▼ fifo_in                ┌──┴────────┬───────┐
│   │      ┌──────────────┐  register_if  │           │       │
│   │      │ Peripheral B │◄──────────────┤ systembus │  CPU  │
│   │      └─┬───┬────────┘               │           │       │
│   │    dma │   │ fifo_out               └──┬────────┴───────┘
│   │        │   │                           │            ▲  IRQ
│   │        │   │                           │            │
│   │ mem_if ▼   ▼ fifo_in   register_if     │            │
│   │      ┌──────────────┐◄─────────────────┘            │
│   │      │ Peripheral C │  transfer_done_irq            │
│   │      └─┬───┬────────┴───────────────────────────────┘
│   │    dma │   │ fifo_out
│   └────────┘   │
└────────────────┘
```

An implementation of this system is included in the `systemc/original-system`
subdirectory.

The above has been translated to a co-simulation with Renode focusing on the
usage of multiple `SystemCPeripheral` components. The minimal SystemC virtual
platform has been substituted with an STM32 F401 mini board model.

The resulting system (equivalent to the above) is as follows:

``` raw

      SystemC process                                   Renode process
  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
   ┌────────────┐           ┌───────────────┐                ┌─────────────────┐                ┌─────────┐
  ││            │ SysC/TLM  │               ││ TCP  │        │                 │                │         │          │
   │Peripheral A├───────────┤ Renode_bridge ├────────────────┤SystemCPeripheral├────────────────┤         │
  ││            │           │               ││      │ ┌─DC1─►│                 │                │         │          │
   └────────────┘           └───────────────┘         │  ┌──►└┬─────────┬──────┘                │         │
  └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘      │ │ DC0   │ DC0     │ DC1                   │         │          │
      SystemC process                                 │  │    │ (DMA)   │ (FIFO)                │         │
  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐      │ │  │    ▼         ▼                       │         │          │
   ┌────────────┐           ┌───────────────┐         │  │   ┌─────────────────┐                │         │
  ││            │ SysC/TLM  │               ││ TCP  │ │  │   │                 │                │         │    ┌───┐ │
   │Peripheral B├───────────┤ Renode_bridge ├ ───────────────┤SystemCPeripheral├────────────────┤SystemBus├────┤CPU│ 
  ││            │           │               ││      │ │  │   │                 │                │         │    └─┬─┘ │
   └────────────┘           └───────────────┘         │  │   └┬─────────┬──────┘                │         │      │
  └──────────────────────────────────────────┘      │ │  │    │ DC0     │ DC1                   │         │      │   │
      SystemC process                                 │  │    │ (DMA)   │ (FIFO)                │         │      │
  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐      │ │  │    ▼         ▼                       │         │      │   │
   ┌────────────┐           ┌───────────────┐         │  │   ┌─────────────────┐                │         │      │
  ││            │ SysC/TLM  │               ││ TCP  │ │  │   │                 │                │         │      │   │
   │Peripheral C├───────────┤ Renode_bridge ├ ───────────────┤SystemCPeripheral├────────────────┤         │      │
  ││            │           │               ││      │ │  │   │                 │ GPIO (for IRQ) │         │      │   │
   └────────────┘           └───────────────┘         │  │   └┬─────────┬──────┴──────────────┐ │         │      │
  └─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘      │ │  │    │DC0      │ DC1                 │ └─────────┘      │   │
                                                      │  │    │(DMA)    │ (FIFO)              │                  │
                                                    │ │  └────┘         │                     └──────────────────┘   │
                                                      └─────────────────┘
                                                    └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
```

All SystemC processes are started from the same executable. Implementation of
the peripherals has not been changed with respect to the original example, with
one minor exception which is explained below.

## Logic flow

The behavior of the system simulated in the example is the same as in the
original version:

1.  CPU starts a timer in Peripheral A by writing to the appropriate
    register through SystemBus.
2.  Timer wraps around, which initializes the data transfer to Peripheral B
    through the FIFO direct connection.
3.  After receiving all the data through FIFO, Peripheral B writes it to
    Peripheral C using the DMA direct connection.
4.  When Peripheral C receives all the data, it raises an interrupt. The CPU
    interrupt handler terminates the example.

There is one minor modification to the original example, where the timer
mentioned in pt. 2 was 64-bit; it has been changed to 32-bit. The reason was
that the original example simulates over 2500 hours of virtual time to achieve
the wrap-around of the 64-bit timer. This can be done instantaneously in
SystemC where the system is idle through all of this time. However, it is not
practical in Renode, which doesn't make assumptions about the future state of
the system and so cannot "skip" the idle time in the same way.
