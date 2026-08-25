### Multi-core

This example illustrates two Renode CPUs (an Arm Cortex-M55 and an Arm Cortex-M0)
sharing a single SystemC process through one `SystemCCortexMSignalsUnit`. The
SystemC side contains only memory.

```raw
                  Renode process                                          SystemC process
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐      ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
  ┌──────────────┐                                           ┌───────────────┐
│ │ CPU.CortexM  │ cpuId: 0                        │      │  │ renode_bridge │ id: 0                        │
  │     m55      ├──────┐                                    │ m_bridge_m55  ├────────┐
│ └──────────────┘      │                          │      │  └───────┬───────┘        │                     │
                        │  ┌─────────────────────┐   TCP     ┌───────┴───────────┐  ┌─┴────────────┐
│                       ├──┤SystemCCortexMSignals├─┼──────┼──┤ renode_connection │  │    Memory    │        │
                        │  │  Unit (cortexMProxy)│           │      m_conn       │  │ m_shared_mem │
│ ┌──────────────┐      │  └─────────────────────┘ │      │  └───────┬───────────┘  └─┬────────────┘        │
  │ CPU.CortexM  │      │                                    ┌───────┴───────┐        │
│ │     m0       ├──────┘                          │      │  │ renode_bridge ├────────┘                     │
  └──────────────┘ cpuId: 1                                  │ m_bridge_m0   │ id: 1
│                                                  │      │  └───────────────┘                              │
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘      └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```

#### How the cores are told apart

Every request Renode sends over the forward socket carries an `initiator_id`,
which is the `MultiprocessingId` of the CPU that issued the access. On the SystemC
side the messages are handled by two different classes:

- `renode_connection` owns the TCP socket pair to the Renode peripheral. There
  is exactly one per `SystemCCortexMSignalsUnit`. It performs the `INIT`
  handshake, handles `TIMESYNC`, and dispatches every other request to a
  `renode_bridge` based on `initiator_id`.
- `renode_bridge` represents a single initiator. Each instance has its own
  `initiator_socket` and stamps its `id` on every
  message it sends back to Renode, so updates end up at the right
  `CortexMBundle`.

Both bridges bind their `initiator_socket` to the same `Memory`, which uses a
`multi_passthrough_target_socket` to accept more than one initiator.

#### Debug accesses

Reads and writes from the Monitor (and `AssembleBlock`) are not issued by a CPU
thread, so `SystemCPeripheral` can send them as `READ_DEBUG`/`WRITE_DEBUG`,
which map to `transport_dbg` in SystemC. Setting `SystemCExecutablePath`
disables this by default, assuming the SystemC module doesn't implement
`transport_dbg`; the `Memory` in this example does, so the platform re-enables
it with `cortexMProxy DisableDebugAccess false`. With it enabled, Monitor
accesses are side-effect free and don't advance SystemC time.

#### Test

`multi-core-read-write.robot` loads small programs into the shared
memory and checks that:

1. A word written through one core's view of the bus is readable through the
   other's, without running any code.
2. Each core posts a magic value to its own mailbox, spins until it sees the
   other core's value, and echoes it back.
3. The cores take strict turns incrementing a shared counter: the M55 only
   increments when the value is even, the M0 only when it is odd. Each core
   keeps a private tally, so the test can verify that both did exactly half of
   the increments.
