#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 4 ]]; then
  echo "Usage: $0 <firmware.hex> <serial_port> <baud_rate> <stm32_programmer_cli_path>"
  echo "Example: $0 ../firmware/telemetry.hex /dev/ttyUSB0 115200 \"$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI\""
  exit 1
fi

FW_HEX="$1"
SERIAL_PORT="$2"
BAUD="$3"
STM32CLI="$4"

if [[ ! -f "${FW_HEX}" ]]; then
  echo "Firmware file not found: ${FW_HEX}" >&2
  exit 1
fi

if [[ ! -x "${STM32CLI}" ]]; then
  echo "STM32_Programmer_CLI not executable at: ${STM32CLI}" >&2
  exit 1
fi

"${STM32CLI}" -c port="${SERIAL_PORT}" br="${BAUD}" -w "${FW_HEX}" -v -rst

echo "Flash complete via UART bootloader."
