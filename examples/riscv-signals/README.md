### RISC-V signals

This example illustrates `SystemC.SystemCRiscVSignalsUnit` connected to a RISC-V CPU.
It is the RISC-V counterpart of the `cortex-m-signals` example and shows how to expose CPU signals and bus interfaces between Renode and SystemC.

The Renode platform contains a `RISC-V` CPU, a PLIC, a CLINT and a `SystemCRiscVSignalsUnit` peripheral.
The signals unit keeps a per-hart `RiscVBundle` with the CPU, its interrupt controller and optionally a timer.
The SystemC process contains a small model that drives the signals and exposes the bus endpoints described below.

#### Exposed signals

The signals are numbered as follows (keep them in sync with `SystemCRiscVSignalsUnit.Signal` and `top.h`):

| ID | Direction | Description |
|----|-----------|-------------|
| 0-4095 | SystemC -> Renode | Interrupt inputs, forwarded to the interrupt controller |
| 4096 | SystemC -> Renode | NMI input |
| 4097 | SystemC -> Renode | Reset input |

`numberOfInterrupts` is the number of input slots starting at zero.
With a PLIC, slot 0 is reserved and should remain unconnected.
This example uses slots 1-15 for its 15 PLIC sources.
Without an interrupt controller, inputs connect directly to CPU interrupt bits and default to the CPU width (32 or 64).

The CPU boot address is sent over a backward message (`SET_BOOT_ADDRESS`) and updates the CPU reset vector.
Before the CPU first starts, it also sets the PC.
Later changes take effect on the next reset without changing the running PC.
The clock period is also sent over a backward message (`SET_CLOCK_FREQUENCY`) and controls the CPU clocked state (and the timer frequency, if the bundle has a timer exposing `IHasFrequency`).
Stopping the CPU clock does not stop the CLINT time base.

Reset holds the CPU until the input is deasserted, then resets and resumes it along with the bundle's interrupt controller and timer.

#### Exposed SystemC sockets

`riscv_signals` in the SystemC part exposes:

- `system_bus_initiator_socket` - SystemC-initiated transactions to Renode's system bus,
- `memory_target_socket` - Renode-initiated transactions to the SystemC memory model.

An additional `control_target_socket` is used by the example to drive the signals from Renode.
Addresses below `0x1000` are routed to the control space and `0x1000-0x1fff` to the memory.
Interrupt inputs are forwarded to the PLIC modelled in Renode.

The control space uses the following offsets:

| Offset | Description |
|--------|-------------|
| `0x000-0x00f` | Interrupt input 0-15 (one byte each; input 0 is reserved by the PLIC) |
| `0x100` | Reset input |
| `0x104` | NMI input |
| `0x108` | Boot address (64-bit) |
| `0x110` | Clock period in nanoseconds (32-bit, 0 disables the clock) |
| `0x114` | Writing a non-zero value triggers a SystemC-initiated write to the Renode system bus |

```raw
                 Renode process                         SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐     ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
       ┌─────────────────────────┐        TCP    ┌─────────────┐  ┌─────────────────┐
│      │ SystemCRiscVSignalsUnit ├──────┼─────┼──┤renode_bridge├──┤  riscv_signals  │ │
       └───────┬─────────────────┘               └─────────────┘  └─────────────────┘
│              │                        │     └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
       ┌───────┴─────────┐
│      │  CPU.RiscV32    │              │
       └─────────────────┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```

Running the example:

```bash
renode-test examples/riscv-signals/riscv-signals.robot
```
