#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <firmware.hex> <stm32_programmer_cli_path>"
  echo "Example: $0 ../firmware/telemetry.hex \"$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI\""
  exit 1
fi

FW_HEX="$1"
STM32CLI="$2"

if [[ ! -f "${FW_HEX}" ]]; then
  echo "Firmware file not found: ${FW_HEX}" >&2
  exit 1
fi

if [[ ! -x "${STM32CLI}" ]]; then
  echo "STM32_Programmer_CLI not executable at: ${STM32CLI}" >&2
  exit 1
fi

"${STM32CLI}" -c port=SWD mode=UR -w "${FW_HEX}" -v -rst

echo "Flash complete via ST-LINK/SWD."
