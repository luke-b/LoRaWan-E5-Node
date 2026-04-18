#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
OUT_DIR="${PROJECT_DIR}/production-artifacts"
BUNDLE_ROOT="${OUT_DIR}/wio-e5-telemetry-production"
FIRMWARE_DIR="${BUNDLE_ROOT}/firmware"
FLASH_DIR="${BUNDLE_ROOT}/flash"
DOCS_DIR="${BUNDLE_ROOT}/docs"
ZIP_FILE="${OUT_DIR}/wio-e5-telemetry-production-bundle.zip"

rm -rf "${BUNDLE_ROOT}"
mkdir -p "${FIRMWARE_DIR}" "${FLASH_DIR}" "${DOCS_DIR}"

# Verify expected production build outputs exist.
for file in telemetry.bin telemetry.hex telemetry.elf telemetry.map; do
  if [[ ! -f "${PROJECT_DIR}/${file}" ]]; then
    echo "[packaging] Missing build output: ${file}" >&2
    echo "[packaging] Run 'make clean all' in ${PROJECT_DIR} first." >&2
    exit 1
  fi
  cp "${PROJECT_DIR}/${file}" "${FIRMWARE_DIR}/"
done

cp "${SCRIPT_DIR}/templates/flash_stlink.sh" "${FLASH_DIR}/"
cp "${SCRIPT_DIR}/templates/flash_uart.sh" "${FLASH_DIR}/"
cp "${SCRIPT_DIR}/templates/flash_uart_windows.bat" "${FLASH_DIR}/"
cp "${SCRIPT_DIR}/templates/FLASHING_GUIDE.txt" "${DOCS_DIR}/"

chmod +x "${FLASH_DIR}/flash_stlink.sh" "${FLASH_DIR}/flash_uart.sh"

# Normalize line endings to LF in generated package content.
for file in "${FLASH_DIR}/flash_stlink.sh" "${FLASH_DIR}/flash_uart.sh" "${DOCS_DIR}/FLASHING_GUIDE.txt"; do
  sed -i 's/\r$//' "${file}"
done

(
  cd "${OUT_DIR}"
  rm -f "${ZIP_FILE}"
  zip -r "${ZIP_FILE}" "wio-e5-telemetry-production" >/dev/null
)

echo "[packaging] Production artifact bundle ready: ${ZIP_FILE}"
