### Cortex-M Signals

This basic example illustrates a SystemCCortexMSignalsUnit peripheral connected to an Arm Cortex-M55 CPU.

From the perspective of the CPU, outgoing signals such as O_sysreset_req update the internal
state of the signals_peripheral. The SystemC state of these signals can be queried by reading from
the offset in SystemCCortexMSignalsUnit that corresponds to the signal ID (e.g. 6 for O_sysreset_req).
These reads are forwarded to the SystemC process, which checks the current state of the corresponding
signal and returns it to Renode.

From Renode's persective, the incoming signals such as cpu_wait must be triggered by SystemC.
In order to get signals_peripheral in SystemC to raise these signals, you can write the signal ID
to offset 0x100.

```raw
                 Renode process                         SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐     ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
       ┌─────────────────────────┐        TCP    ┌─────────────┐  ┌───────────────────────┐
│      │SystemCCortexMSignalsUnit├──────┼─────┼──┤renode_bridge├──┤  signals_peripheral   │ │
       └───────┬─────────────────┘               └─────────────┘  └───────────────────────┘
│              │                        │     └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
       ┌───────┴─────────┐
│      │  CPU.CortexM    │              │
       └─────────────────┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```
