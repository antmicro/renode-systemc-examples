#!/usr/bin/env bash

set -o errexit  # abort on nonzero exitstatus
set -o nounset  # abort on unbound variable
set -o pipefail # don't hide errors within pipes

toolchain="arm-zephyr-eabi"
os_type="$(uname -s)"

./scripts/fetch_zephyr_sdk.sh

# Set up Zephyr SDK
pushd zephyr-sdk
case "${os_type}" in
Linux*)
  ./setup.sh -t "${toolchain}" -h -c
  export ZEPHYR_SDK_INSTALL_DIR=$(pwd)
  ;;
MINGW* | MSYS* | CYGWIN*)
  setup_cmd="$(cygpath -w "$(pwd)/setup.cmd")"
  cmd.exe //C "call ${setup_cmd} /t ${toolchain} /h /c"
  export ZEPHYR_SDK_INSTALL_DIR="$(cygpath -w "$(pwd)")"
  ;;
*)
  echo "Unknown operating system: ${os_type}" >&2
  exit 1
  ;;
esac
popd

# Install West
pip install west

# Install Zephyr into this repo.
west init -l . --mf west.yml

# Updating may fail, so retry five times.
for i in $(seq 1 5); do
  echo try "$i" of 5
  west update >>/dev/null 2>&1 && break || sleep 5
done

west zephyr-export
west packages pip --install

mkdir -p artifacts/zephyr_binaries

for example in examples/*/zephyr; do
  example_name=$(echo "$example" | cut -f 2 -d '/')
  west build -p -b stm32f401_mini "$example"
  cp build/zephyr/zephyr.elf artifacts/zephyr_binaries/"$example_name".elf
done
