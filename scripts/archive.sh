#!/usr/bin/env bash

set -o errexit  # abort on nonzero exitstatus
set -o nounset  # abort on unbound variable
set -o pipefail # don't hide errors within pipes

mkdir -p "$TARGET_DIR"

for entry in "$@"; do
  echo "$entry"
  if [ -e "$entry" ]; then
    cp -v -r "$entry" "$TARGET_DIR"
  fi
done
