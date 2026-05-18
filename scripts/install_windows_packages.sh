#!/usr/bin/env bash

set -o errexit  # abort on nonzero exitstatus
set -o nounset  # abort on unbound variable
set -o pipefail # don't hide errors within pipes

if [ "$#" -lt 1 ]; then
  echo "error: missing path to Windows packages config file" >&2
  echo "usage: $0 <windows-packages.config>" >&2
  exit 1
fi

windows_packages_config="$1"

if [ -n "${WINDOWS_BUILD_TYPE:-}" ]; then
  # Filter which dependencies are needed based on the build type.
  sed -i /except:"$WINDOWS_BUILD_TYPE"/d "$windows_packages_config"
fi

retry_windows_package_install() {
  local max_attempts=3
  local sleep_time=60s
  local exit_code=1

  for i in $(seq 1 "${max_attempts}"); do
    echo "Install Windows packages attempt ${i} out of ${max_attempts}"

    if choco install ./scripts/windows_packages.config --force -y --no-progress; then
      echo "Windows packages have been installed"
      return 0
    else
      exit_code=$?
      echo "Failed to install Windows packages (exit code ${exit_code})"
    fi

    if [ "${i}" -lt "${max_attempts}" ]; then
      sleep "${sleep_time}"
    fi
  done

  return "${exit_code}"
}

# The choco install has been observed to fail at times, due to errors about other installs running.
# We retry in hopes that the other installs finish and let us install our packages.
retry_windows_package_install
