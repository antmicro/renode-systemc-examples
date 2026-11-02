### Heterogeneous CPU signals

This example connects one RV32 core and one ARM Cortex-M4 core to a single `SystemC.SystemCCPUSignalsUnit`.
Its `bundles` list contains a `RiscVBundle` (CPU, PLIC, CLINT) and a `CortexMBundle` (CPU, NVIC, DWT).

The SystemC model creates one `renode_connection` and passes it to two `renode_bridge` instances.
Each bridge's initiator ID must match its Renode CPU's multiprocessing ID: RISC-V `hartId: 0`, ARM `cpuId: 1`.
The IDs must be unique across all bundles, including CPUs of different architectures.

Both CPUs map the signals unit at `0x60000000`, using CPU-specific bus registrations.
Initiator IDs route accesses to separate SystemC control and memory banks at the same address.
The tests execute RISC-V and Thumb code from these banks simultaneously, and verify that resetting or stopping one core leaves the other running.

#### Exposed signals

The signals are numbered as follows (keep them in sync with `SystemCRiscVSignalsUnit.Signal`, `SystemCCortexMSignalsUnit.Signal`, and `top.cpp`):

| Signal | RISC-V bridge (ID 0) | ARM bridge (ID 1) |
|--------|----------------------|-------------------|
| Interrupt inputs | 1–15 (PLIC source 0 is reserved) | 0–15 (NVIC external IRQs) |
| NMI | 4096 | 1000 |
| Reset | 4097 | 1001 (`CoreResetIn`) |
| CPUWAIT | n/a | 1002 |
| System reset request, Renode to SystemC | n/a | 1006 |
| Boot configuration | `boot_address_in` | `init_vtor_ns_in` |
| Clock period | `clock_period_in` | `clock_period_in` |

#### Exposed SystemC sockets

`signals` in the SystemC part exposes:

- `system_bus_initiator_socket` - SystemC-initiated transactions to Renode's system bus,
- `memory_target_socket` - Renode-initiated transactions to the SystemC memory model.

An additional `control_target_socket` is used by the example to drive the signals from Renode.
Interrupt inputs are forwarded to the PLIC and NVIC modelled in Renode.

The following control offsets are relative to `0x60000000`.
Use `context=riscv` or `context=arm` in Monitor bus accesses to choose the bridge.
Direct accesses to the signals peripheral that lack a CPU context default to initiator 0.

| Offset | Access | Meaning |
|--------|--------|---------|
| `0x000–0x00f` | byte | Interrupt input levels |
| `0x100` | byte | Active-high reset |
| `0x104` | byte | NMI level |
| `0x108` | RV: 64-bit, ARM: 32-bit | RISC-V reset vector or ARM vector table address |
| `0x110` | 32-bit | Clock period in nanoseconds, zero stops the core clock |
| `0x114` | 32-bit | Nonzero triggers a SystemC write to Renode RAM |
| `0x118` | byte | ARM CPUWAIT level |
| `0x11c` | byte, read-only | ARM system reset request level |
| `0x1000–0x1fff` | memory | Per-core SystemC memory, also executable |

RISC-V starts with a 10 ns clock period and reset vector `0x80000000`.
ARM uses 20 ns and vector table `0x20000000`.

Writing the trigger register `0x114` stores `0x5a5aa5a5` at `0x80001000` for RISC-V, or `0x5a5aa5a6` at `0x20001000` for ARM.

Building the example:

```bash
cmake --build build --target hmp-signals
mkdir -p examples/hmp-signals/bin
cp build/examples/hmp-signals/hmp-signals examples/hmp-signals/bin/
```

Running the example:

```bash
renode-test examples/hmp-signals/hmp-signals.robot
```
