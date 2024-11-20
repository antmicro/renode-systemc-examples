# Renode - SystemC integration examples

Copyright (c) 2024 [Antmicro](https://www.antmicro.com)

## Overview

This repository contains tools for creating Renode simulations using
components written in SystemC and a range of examples.

Renode and SystemC simulations run as separate processes, communicating
through reusable interface components - a memory-mapped `SystemCPeripheral`
peripheral in Renode and a `renode_bridge` SystemC module in SystemC.

From the perspective of Renode, a SystemC model can be interacted with like any
other peripheral. From the perspective of SystemC, Renode is represented
by a module connected by standard TLM sockets.

## Dependencies

-   Renode
-   SystemC dynamic libraries
-   C++ compiler (tested with Clang, GCC)
-   CMake

## Building the examples

The repository follows the standard CMake build process, that is:
``` bash
$ mkdir build
$ cd build
$ cmake .. -DUSER_RENODE_DIR=<absolute path to Renode>
$ make
```

When run from the repository root, the above commands will build all the examples.
Individual examples are built by running the above in the directory for
a particular example (e.g. `examples/lt`).

### SystemC distribution and C++ standard version

SystemC libraries are compiled against a particular version of the C++
standard. The build process in this repository assumes C++ 17 by default, which
should be the case for all SystemC packages installed on reasonably
up-to-date systems. If this is not the case, however, the builds may fail with
`undefined reference to sc_core::sc_api_version_2_3_4_cxx2017...` linker
errors. In that case, either install an up-to-date SystemC distribution
or pass an appropriate `-DCMAKE_CXX_STANDARD` argument to the relevant `cmake`
call, for example:

``` bash
$ cmake .. -DUSER_RENODE_DIR=<absolute path to Renode> -DCMAKE_CXX_STANDARD=14
```

This will build `renode_bridge` and the examples using the chosen
standard instead of the default C++17.

## Repository structure

### `examples` directory

Contains one subdirectory for each available example, with the following
structure:

``` raw
my_example
├── CMakeLists.txt           CMake configuration building the SystemC executable (bin/my_example)
├── bin
│   ├── my_example           SystemC executable
│   └── my_example.elf       Zephyr binary, if the example uses it and is built locally
├── renode
│   ├── my_example.repl      Description of the Renode platform on which the example is run
│   └── my_example.resc      Renode script for initialization of the example 
├── systemc
│   ├── include              C++/SystemC header files for the example
│   └── src                  C++/SystemC implementation files for the example
├── zephyr                   Zephyr RTOS source code, if the example uses a Zephyr executable
└── my_example.robot         Robot Framework file with test logic
```

### `tests` directory

Contains tests for specific scenarios, including regression tests. They are not very illustrative and less interesting to users looking for reference. Other than that, the structure is identical to that of `examples`.

## Running the examples

To run `my_example` from the repository root, use:

```bash
$ renode-test examples/my_example/my_example.robot
```

To run all examples:

``` bash
$ pushd examples
$ renode-test -t all_examples.yaml
$ popd
```

## Example descriptions

### [Loosely-timed SystemC example](examples/lt)

Located in the /examples/lt folder, `lt` illustrates the integration of Renode with a non-trivial SystemC
model. It is an adaptation of the `loosely-timed` example, which is part of the
official SystemC-2.3.4 distribution.

The SystemC model consists of two `initiator` modules that generate traffic, two
`memory` modules that implement simple memories and a bus module that connects
them.

The above has been extended so that the Renode simulation serves as a third
"initiator" module, reading and writing to the memories modelled in the
SystemC process via `sysbus Read/Write ...` Renode commands.

### [Time synchronization on STM32 Mini F401](examples/timesync/)

Located in the /examples/timesync folder, `timesync` illustrates virtual time synchronization between Renode and
SystemC. It is also simulated within a more complex platform (the [STM32 Mini
F401](https://renodepedia.renode.io/boards/stm32f401_mini/?view=software&demo=hello_world)),
extended with the SystemC peripheral. The SystemC part is a simple timer that
returns SystemC virtual time in seconds when read from. The Renode simulation
reads it and outputs the obtained time to the UART. It can then be compared with
Renode virtual time to confirm that they are synchronized.

### [Transaction delay on STM32 Mini F401](examples/transaction-delay)

Located in the /examples/transaction-delay folder, `transaction-delay` is very similar to `timesync`, but additionally the
read transaction from the SystemC peripheral is delayed through the `delay`
parameter in the TLM `b_transport` call. The result is that time value read
from `timekeeper` arrives with a one second delay, which can be confirmed by
looking at the UART output.

### [Interrupts and GPIO](examples/interrupts)

Located in the /examples/interrupts folder, `interrupts`  shows how to model interrupt requests sent through the Renode -
SystemC interface.

The SystemC part contains two `interrupter` modules, each sending IRQs to Renode.
`Interrupters` are connected by a simple bus module, routing the requests
to one or the other based on the address.

Interrupter modules connect their interrupt lines directly to `renode_bridge`
through SystemC ports bound to `sc_signal<bool>` channels.

The two interrupter modules raise interrupts in fixed periods. The interrupt
signals are cleared when any value is written to the interrupters. The Renode side
of the simulation sets up two interrupt handlers - one for each interrupter.
When the Renode-simulated CPU receives an interrupt, an output message is
written to the UART and the interrupt signal is cleared by writing to the
appropriate interrupter memory address.

### [Simple DMA](examples/dma)

Located in the /examples/dma folder, `dma` shows a rudimentary DMA controller (DMAC) peripheral implemented in
SystemC and connected to Renode. It illustrates SystemC-to-Renode bus
communication and handling of Renode-initiated GPIO signals.

The Renode script sets up the DMAC by setting the appropriate source, destination
and data length registers. It then starts the memory transfer by writing to the
DMAC control register. Next, DMAC is informed that the bus is free by Renode
raising a "bus free" signal on one of the GPIO pins on the SystemC
peripheral. The DMAC then performs a memory-to-memory transfer according to the
setup.

### [Direct connection](examples/direct-connection)

Located in the /examples/direct-connection folder, `direct-connection` illustrates a case where there are multiple SystemC peripherals.
It also demonstrates the use of the `direct connection` functionality, allowing
peripherals to communicate using memory-mapped transfers that are not going through the
Renode system bus. This, for example, allows them to communicate using a
different address space than the system bus address space.

The peripherals hold a single value, which can be changed by writing to the
peripheral. When read from, the peripheral will read the value not from itself,
but from the other peripheral, using the direct connection. This is transparent from
the point of view of the Renode system bus. The example works by writing values
to the two peripherals, and illustrating that reading from one of them returns
the value saved in the other.

``` raw
┌─────────────────┐                  ┌────────────┐
│                 │◄────────────────►│Peripheral 0│
│                 │                  └─────┬──────┘
│Renode system bus│                        │ Direct connection
│                 │                  ┌─────┴──────┐
│                 │◄────────────────►│Peripheral 1│
└─────────────────┘                  └────────────┘
```

### [Non-blocking TLM binding](examples/tlm-non-blocking)

Located in the /examples/tlm-non-blocking folder, `tlm-non-blocking` is almost identical to `direct-connection`. The most important
difference is that the peripherals use a non-blocking TLM interface. This is just
a matter of syntax, as the integration still handles the transactions in a
single phase.

### [Multiple peripherals](examples/multiple-peripherals)

Located in the /examples/multiple-peripherals folder, `multiple-peripherals` illustrates a possible translation of a pure SystemC simulation to
a co-simulation with Renode, with use of most integration features. The README in
the example directory provides more details.

### [GPIO connection](examples/gpio-connection)

Located in the /examples/gpio-connection folder, this basic example illustrates a SystemCPeripheral connected to an output GPIO of another peripheral.

The SystemC model updates its internal state after receiving a GPIO event, sent by a `MiV_CoreGPIO` peripheral modelled in Renode. The state is then read
through the system bus to verify that the communication finished successfully.
