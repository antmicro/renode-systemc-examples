# SystemC `b_transport` benchmark

This benchmark compares Renode's existing trivial mock peripheral with a
trivial SystemC target. Writes are ignored, reads return zero, and each loop
iteration performs one write and one read. DMI is disabled, so SystemC accesses
go through `b_transport`.

`PERIPHERALS` may be 1, 2, or 4. It distributes the same transaction count over
that many Renode bus registrations or SystemC address windows.

## Build and run

The Renode source tree must contain `librenode`, which can be built with:

```sh
./build.sh --shared
```

from the Renode source directory.

Then, from this benchmark directory:

```sh
cmake -B build -DUSER_RENODE_DIR=/path/to/renode -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

/path/to/renode/renode-test benchmark.robot --include renode --variable ITERATIONS:500000 --variable PERIPHERALS:1
/path/to/renode/renode-test benchmark.robot --include systemc --variable ITERATIONS:500000 --variable PERIPHERALS:1
```

The first test uses the normal Renode process created by `renode-test`. The
second launches `build/bin/systemc_trivial`, which hosts its own Renode instance
with `librenode`. Repeat with `PERIPHERALS:4` to check performance with multiple windows.

To change the access pattern, edit `renode/workload.resc`.
Keep the guest instruction and transaction counts identical between tags.

## Profile

On Linux, enable .NET JIT maps and profile one tag per run:

```sh
DOTNET_PerfMapEnabled=3 perf record -F 199 -g -k 1 -o /tmp/native-systemc.data -- \
  build/bin/systemc_trivial renode/systemc.resc 500000 4

perf inject --jit \
  -i /tmp/native-systemc.data \
  -o /tmp/native-systemc.jit.data

perf report -i /tmp/native-systemc.jit.data
```

Use `--include renode` and a different output filename for the baseline.
