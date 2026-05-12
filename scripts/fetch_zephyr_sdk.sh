#!/usr/bin/env bash
# shellcheck disable=SC2329 # It can't detect that they're used through `retry_download_and_extract`

set -o errexit  # abort on nonzero exitstatus
set -o nounset  # abort on unbound variable
set -o pipefail # don't hide errors within pipes

max_attempts=5
sdk_dir="zephyr-sdk"
toolchain="arm-zephyr-eabi"
os_type="$(uname -s)"
curl_args=(
  --insecure
  --fail
  --location
  --show-error
  --progress-bar
)

extract_linux_sdk() {
  tar xJf "${sdk_archive_name}" --strip 1 --directory "${sdk_dir}"
}

extract_windows_sdk() {
  7z x -y "${sdk_archive_name}" "-o${sdk_dir}" || return $?
  normalize_windows_sdk_dir
}

normalize_windows_sdk_dir() {
  local extracted_dir="${sdk_dir}/zephyr-sdk-${ZEPHYR_SDK_VERSION}"

  if [ -d "${extracted_dir}" ]; then
    shopt -s dotglob nullglob
    mv "${extracted_dir}"/* "${sdk_dir}/"
    rmdir "${extracted_dir}"
  fi
}

extract_toolchain() {
  "${extract_toolchain_cmd[@]}"
}

case "${os_type}" in
Linux*)
  sdk_archive_name="zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz"
  toolchain_archive_name="toolchain_linux-x86_64_${toolchain}.tar.xz"

  extract_sdk=extract_linux_sdk
  extract_toolchain_cmd=(tar xJf "${toolchain_archive_name}" --directory "${sdk_dir}")
  ;;
MINGW* | MSYS* | CYGWIN*)
  sdk_archive_name="zephyr-sdk-${ZEPHYR_SDK_VERSION}_windows-x86_64_minimal.7z"
  toolchain_archive_name="toolchain_windows-x86_64_${toolchain}.7z"

  extract_sdk=extract_windows_sdk
  extract_toolchain_cmd=(7z x -y "${toolchain_archive_name}" "-o${sdk_dir}")
  ;;
*)
  echo "Unknown operating system: ${os_type}" >&2
  exit 1
  ;;
esac

base_url="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}"

download_archive() {
  local archive_name="$1"
  local url="${base_url}/${archive_name}"

  rm -f "${archive_name}" "${archive_name}.partial"

  echo "Downloading ${archive_name}"
  curl "${curl_args[@]}" \
    --output "${archive_name}.partial" \
    "${url}" || return $?

  mv "${archive_name}.partial" "${archive_name}" || return $?
  echo "Downloaded ${archive_name}"
}

retry_download_and_extract() {
  local description="$1"
  local archive_name="$2"
  local clean_sdk_dir="$3"
  local extract_function="$4"

  for i in $(seq 1 "${max_attempts}"); do
    echo "Download ${description} attempt ${i} out of ${max_attempts}"

    if [ "${clean_sdk_dir}" = "true" ]; then
      rm -rf "${sdk_dir:?}"/*
    fi

    if download_archive "${archive_name}" && "${extract_function}"; then
      echo "${description} has been downloaded and extracted"
      return 0
    fi

    echo "Failed to download/extract ${description}"
    sleep 5s
  done

  return 1
}

mkdir -p "${sdk_dir}"

retry_download_and_extract \
  "Zephyr SDK" \
  "${sdk_archive_name}" \
  true \
  "${extract_sdk}"

retry_download_and_extract \
  "Zephyr SDK toolchain" \
  "${toolchain_archive_name}" \
  false \
  extract_toolchain

echo "zephyr-sdk has been downloaded"
exit 0
