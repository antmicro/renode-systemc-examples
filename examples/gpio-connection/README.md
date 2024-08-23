### GPIO connection

This basic example illustrates a SystemCPeripheral connected to an output GPIO of another peripheral.

The SystemC model updates its internal state after receiving a GPIO event, sent by a `MiV_CoreGPIO` peripheral modelled in Renode. The state is then read through the system bus to verify that the communication finished successfully.

``` raw
                 Renode process                         SystemC process   
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐     ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
       ┌─────────────────┐        TCP    ┌─────────────┐  ┌───────────────────────┐  
│      │SystemCPeripheral├──────┼─────┼──┤renode_bridge├──┤    gpio_peripheral    │ │
       └───────┬─────────┘               └─────────────┘  └───────────────────────┘  
│              │                │     └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
       ┌───────┴─────────┐
│      │  MiV_CoreGPIO   │      │
       └─────────────────┘
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
```
