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

SE_IDENTITY_FILE="${PROJECT_DIR}/LoRaWAN/App/se-identity.h"
LORA_APP_FILE="${PROJECT_DIR}/LoRaWAN/App/lora_app.h"
TELEMETRY_APP_FILE="${PROJECT_DIR}/Core/Inc/telemetry_app.h"

SAMPLE_LORAWAN_DEVICE_EUI="0080E10101010101"
SAMPLE_LORAWAN_JOIN_EUI="0101010101010101"
SAMPLE_LORAWAN_APP_KEY="2B7E151628AED2A6ABF7158809CF4F3C"
SAMPLE_LORAWAN_NWK_KEY="2B7E151628AED2A6ABF7158809CF4F3C"

extract_define_value() {
  local file_path="$1"
  local macro_name="$2"
  local line
  line="$(grep -E "^#define[[:space:]]+${macro_name}[[:space:]]+" "${file_path}" | head -n 1 || true)"
  if [[ -z "${line}" ]]; then
    return 1
  fi

  line="${line#*${macro_name}}"
  line="$(echo "${line}" | sed -E 's@/\*.*\*/@@g; s@//.*$@@; s/^[[:space:]]+//; s/[[:space:]]+$//')"
  printf "%s" "${line}"
}

normalize_hex() {
  local value="$1"
  value="$(echo "${value}" | tr '[:lower:]' '[:upper:]')"
  value="$(echo "${value}" | sed -E 's/0X//g; s/[^0-9A-F]//g')"
  printf "%s" "${value}"
}

manifest_value_or_default() {
  local env_name="$1"
  local fallback_value="$2"
  local value="${!env_name:-}"

  if [[ -n "${value}" ]]; then
    printf "%s" "${value}"
    return
  fi

  printf "%s" "${fallback_value}"
}

fail() {
  local message="$1"
  echo "[packaging] ERROR: ${message}" >&2
  exit 1
}

validate_hex_length() {
  local field_name="$1"
  local field_value="$2"
  local expected_length="$3"

  if [[ ! "${field_value}" =~ ^[0-9A-F]+$ ]]; then
    fail "${field_name} must contain only hex characters [0-9A-F], got '${field_value}'."
  fi

  if [[ "${#field_value}" -ne "${expected_length}" ]]; then
    fail "${field_name} must have length ${expected_length}, got ${#field_value}."
  fi
}

validate_decimal_field() {
  local field_name="$1"
  local field_value="$2"

  if [[ ! "${field_value}" =~ ^[0-9]+$ ]]; then
    fail "${field_name} must be a decimal number, got '${field_value}'."
  fi
}

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

DEFAULT_DEV_EUI_RAW="$(extract_define_value "${SE_IDENTITY_FILE}" "LORAWAN_DEVICE_EUI")"
DEFAULT_JOIN_EUI_RAW="$(extract_define_value "${SE_IDENTITY_FILE}" "LORAWAN_JOIN_EUI")"
DEFAULT_APP_KEY_RAW="$(extract_define_value "${SE_IDENTITY_FILE}" "LORAWAN_APP_KEY")"
DEFAULT_NWK_KEY_RAW="$(extract_define_value "${SE_IDENTITY_FILE}" "LORAWAN_NWK_KEY")"
DEFAULT_REGION_RAW="$(extract_define_value "${LORA_APP_FILE}" "ACTIVE_REGION")"
DEFAULT_ACTIVATION_RAW="$(extract_define_value "${LORA_APP_FILE}" "LORAWAN_DEFAULT_ACTIVATION_TYPE")"
DEFAULT_DUTY_CYCLE_RAW="$(extract_define_value "${LORA_APP_FILE}" "APP_TX_DUTYCYCLE")"
DEFAULT_APP_PORT_RAW="$(extract_define_value "${LORA_APP_FILE}" "LORAWAN_USER_APP_PORT")"
DEFAULT_ADR_RAW="$(extract_define_value "${LORA_APP_FILE}" "LORAWAN_ADR_STATE")"
DEFAULT_DATA_RATE_RAW="$(extract_define_value "${LORA_APP_FILE}" "LORAWAN_DEFAULT_DATA_RATE")"
DEFAULT_PAYLOAD_SIZE_RAW="$(extract_define_value "${TELEMETRY_APP_FILE}" "TELEMETRY_PAYLOAD_SIZE")"

MANIFEST_DEVICE_ID="$(manifest_value_or_default "TELEMETRY_DEVICE_ID" "UNSPECIFIED_DEVICE")"
MANIFEST_SERIAL_NUMBER="$(manifest_value_or_default "TELEMETRY_SERIAL_NUMBER" "UNSPECIFIED_SERIAL")"
MANIFEST_BUSINESS_PROFILE="$(manifest_value_or_default "TELEMETRY_BUSINESS_PROFILE" "default")"

MANIFEST_LORAWAN_DEVICE_EUI="$(normalize_hex "$(manifest_value_or_default "TELEMETRY_LORAWAN_DEVICE_EUI" "${DEFAULT_DEV_EUI_RAW}")")"
MANIFEST_LORAWAN_JOIN_EUI="$(normalize_hex "$(manifest_value_or_default "TELEMETRY_LORAWAN_JOIN_EUI" "${DEFAULT_JOIN_EUI_RAW}")")"
MANIFEST_LORAWAN_APP_KEY="$(normalize_hex "$(manifest_value_or_default "TELEMETRY_LORAWAN_APP_KEY" "${DEFAULT_APP_KEY_RAW}")")"
MANIFEST_LORAWAN_NWK_KEY="$(normalize_hex "$(manifest_value_or_default "TELEMETRY_LORAWAN_NWK_KEY" "${DEFAULT_NWK_KEY_RAW}")")"

MANIFEST_REGION="$(manifest_value_or_default "TELEMETRY_LORAWAN_REGION" "${DEFAULT_REGION_RAW}")"
MANIFEST_ACTIVATION="$(manifest_value_or_default "TELEMETRY_LORAWAN_ACTIVATION" "${DEFAULT_ACTIVATION_RAW}")"
MANIFEST_DUTY_CYCLE_MS="$(manifest_value_or_default "TELEMETRY_HEARTBEAT_MS" "${DEFAULT_DUTY_CYCLE_RAW}")"
MANIFEST_APP_PORT="$(manifest_value_or_default "TELEMETRY_LORAWAN_APP_PORT" "${DEFAULT_APP_PORT_RAW}")"
MANIFEST_ADR_STATE="$(manifest_value_or_default "TELEMETRY_LORAWAN_ADR" "${DEFAULT_ADR_RAW}")"
MANIFEST_DATA_RATE="$(manifest_value_or_default "TELEMETRY_LORAWAN_DATA_RATE" "${DEFAULT_DATA_RATE_RAW}")"
MANIFEST_PAYLOAD_SIZE="$(manifest_value_or_default "TELEMETRY_PAYLOAD_SIZE" "${DEFAULT_PAYLOAD_SIZE_RAW}")"

MANIFEST_BUILD_PROFILE="${BUILD_PROFILE:-default}"
MANIFEST_GIT_SHA="${GITHUB_SHA:-$(git -C "${PROJECT_DIR}" rev-parse HEAD 2>/dev/null || echo unknown)}"
MANIFEST_GIT_REF="${GITHUB_REF_NAME:-$(git -C "${PROJECT_DIR}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)}"
MANIFEST_CI_RUN_ID="${GITHUB_RUN_ID:-local}"
MANIFEST_GENERATED_AT="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
MANIFEST_TOOLCHAIN="$(arm-none-eabi-gcc --version 2>/dev/null | head -n 1 || echo "arm-none-eabi-gcc unavailable")"

ENFORCE_DEVICE_METADATA="${TELEMETRY_ENFORCE_DEVICE_METADATA:-1}"
ENFORCE_CUSTOM_KEYS="${TELEMETRY_ENFORCE_CUSTOM_KEYS:-0}"

validate_hex_length "TELEMETRY_LORAWAN_DEVICE_EUI" "${MANIFEST_LORAWAN_DEVICE_EUI}" 16
validate_hex_length "TELEMETRY_LORAWAN_JOIN_EUI" "${MANIFEST_LORAWAN_JOIN_EUI}" 16
validate_hex_length "TELEMETRY_LORAWAN_APP_KEY" "${MANIFEST_LORAWAN_APP_KEY}" 32
validate_hex_length "TELEMETRY_LORAWAN_NWK_KEY" "${MANIFEST_LORAWAN_NWK_KEY}" 32

if [[ "${MANIFEST_LORAWAN_DEVICE_EUI}" == "${MANIFEST_LORAWAN_JOIN_EUI}" ]]; then
  fail "Device EUI and Join EUI must be different for unique commissioning."
fi

validate_decimal_field "TELEMETRY_HEARTBEAT_MS" "${MANIFEST_DUTY_CYCLE_MS}"
validate_decimal_field "TELEMETRY_LORAWAN_APP_PORT" "${MANIFEST_APP_PORT}"

if [[ "${ENFORCE_DEVICE_METADATA}" == "1" ]]; then
  if [[ "${MANIFEST_DEVICE_ID}" == "UNSPECIFIED_DEVICE" || -z "${MANIFEST_DEVICE_ID}" ]]; then
    fail "TELEMETRY_DEVICE_ID must be set for production packaging."
  fi

  if [[ "${MANIFEST_SERIAL_NUMBER}" == "UNSPECIFIED_SERIAL" || -z "${MANIFEST_SERIAL_NUMBER}" ]]; then
    fail "TELEMETRY_SERIAL_NUMBER must be set for production packaging."
  fi
fi

if [[ "${ENFORCE_CUSTOM_KEYS}" == "1" ]]; then
  if [[ "${MANIFEST_LORAWAN_DEVICE_EUI}" == "${SAMPLE_LORAWAN_DEVICE_EUI}" ]]; then
    fail "Custom TELEMETRY_LORAWAN_DEVICE_EUI is required (default value is not allowed)."
  fi

  if [[ "${MANIFEST_LORAWAN_JOIN_EUI}" == "${SAMPLE_LORAWAN_JOIN_EUI}" ]]; then
    fail "Custom TELEMETRY_LORAWAN_JOIN_EUI is required (default value is not allowed)."
  fi

  if [[ "${MANIFEST_LORAWAN_APP_KEY}" == "${SAMPLE_LORAWAN_APP_KEY}" ]]; then
    fail "Custom TELEMETRY_LORAWAN_APP_KEY is required (default value is not allowed)."
  fi

  if [[ "${MANIFEST_LORAWAN_NWK_KEY}" == "${SAMPLE_LORAWAN_NWK_KEY}" ]]; then
    fail "Custom TELEMETRY_LORAWAN_NWK_KEY is required (default value is not allowed)."
  fi
fi

export MANIFEST_DEVICE_ID
export MANIFEST_SERIAL_NUMBER
export MANIFEST_BUSINESS_PROFILE
export MANIFEST_LORAWAN_DEVICE_EUI
export MANIFEST_LORAWAN_JOIN_EUI
export MANIFEST_LORAWAN_APP_KEY
export MANIFEST_LORAWAN_NWK_KEY
export MANIFEST_REGION
export MANIFEST_ACTIVATION
export MANIFEST_DUTY_CYCLE_MS
export MANIFEST_APP_PORT
export MANIFEST_ADR_STATE
export MANIFEST_DATA_RATE
export MANIFEST_PAYLOAD_SIZE
export MANIFEST_BUILD_PROFILE
export MANIFEST_GIT_SHA
export MANIFEST_GIT_REF
export MANIFEST_CI_RUN_ID
export MANIFEST_GENERATED_AT
export MANIFEST_TOOLCHAIN
export MANIFEST_FIRMWARE_DIR="${FIRMWARE_DIR}"
export MANIFEST_DOCS_DIR="${DOCS_DIR}"

python3 <<'PY'
import hashlib
import json
import os
from pathlib import Path

firmware_dir = Path(os.environ["MANIFEST_FIRMWARE_DIR"])
docs_dir = Path(os.environ["MANIFEST_DOCS_DIR"])

def hash_file(path: Path) -> str:
  digest = hashlib.sha256()
  with path.open("rb") as handle:
    for chunk in iter(lambda: handle.read(65536), b""):
      digest.update(chunk)
  return digest.hexdigest()

def mask(value: str, head: int, tail: int) -> str:
  if not value:
    return "UNSET"
  if len(value) <= head + tail:
    return "*" * len(value)
  return f"{value[:head]}{'*' * (len(value) - head - tail)}{value[-tail:]}"

firmware_artifacts = []
for file_name in ["telemetry.bin", "telemetry.hex", "telemetry.elf", "telemetry.map"]:
  file_path = firmware_dir / file_name
  firmware_artifacts.append(
    {
      "name": file_name,
      "path": f"firmware/{file_name}",
      "size_bytes": file_path.stat().st_size,
      "sha256": hash_file(file_path),
    }
  )

full_manifest = {
  "schema_version": "1.0",
  "manifest_generated_utc": os.environ["MANIFEST_GENERATED_AT"],
  "build": {
    "profile": os.environ["MANIFEST_BUILD_PROFILE"],
    "git_sha": os.environ["MANIFEST_GIT_SHA"],
    "git_ref": os.environ["MANIFEST_GIT_REF"],
    "ci_run_id": os.environ["MANIFEST_CI_RUN_ID"],
    "toolchain": os.environ["MANIFEST_TOOLCHAIN"],
  },
  "device": {
    "device_id": os.environ["MANIFEST_DEVICE_ID"],
    "serial_number": os.environ["MANIFEST_SERIAL_NUMBER"],
  },
  "lorawan": {
    "region": os.environ["MANIFEST_REGION"],
    "activation": os.environ["MANIFEST_ACTIVATION"],
    "device_eui": os.environ["MANIFEST_LORAWAN_DEVICE_EUI"],
    "join_eui": os.environ["MANIFEST_LORAWAN_JOIN_EUI"],
    "app_key": os.environ["MANIFEST_LORAWAN_APP_KEY"],
    "nwk_key": os.environ["MANIFEST_LORAWAN_NWK_KEY"],
    "app_port": os.environ["MANIFEST_APP_PORT"],
    "adr_state": os.environ["MANIFEST_ADR_STATE"],
    "data_rate": os.environ["MANIFEST_DATA_RATE"],
  },
  "business_logic": {
    "profile": os.environ["MANIFEST_BUSINESS_PROFILE"],
    "heartbeat_period_ms": os.environ["MANIFEST_DUTY_CYCLE_MS"],
    "payload_size_bytes": os.environ["MANIFEST_PAYLOAD_SIZE"],
    "test_mode_enabled": True,
    "message_types": {
      "heartbeat": "0x01",
      "door_alarm": "0x02",
      "water_alarm": "0x03",
      "test": "0x04",
    },
  },
  "artifacts": firmware_artifacts,
}

redacted_manifest = json.loads(json.dumps(full_manifest))
redacted_manifest["lorawan"]["device_eui"] = mask(redacted_manifest["lorawan"]["device_eui"], 4, 4)
redacted_manifest["lorawan"]["join_eui"] = mask(redacted_manifest["lorawan"]["join_eui"], 4, 4)
redacted_manifest["lorawan"]["app_key"] = mask(redacted_manifest["lorawan"]["app_key"], 4, 4)
redacted_manifest["lorawan"]["nwk_key"] = mask(redacted_manifest["lorawan"]["nwk_key"], 4, 4)

(docs_dir / "build_manifest_full.json").write_text(json.dumps(full_manifest, indent=2) + "\n", encoding="utf-8")
(docs_dir / "build_manifest_redacted.json").write_text(json.dumps(redacted_manifest, indent=2) + "\n", encoding="utf-8")

summary_lines = [
  "Telemetry Build Manifest Summary",
  "===============================",
  f"Generated UTC: {full_manifest['manifest_generated_utc']}",
  f"Build: {full_manifest['build']['git_sha']} ({full_manifest['build']['git_ref']})",
  f"Device ID: {full_manifest['device']['device_id']}",
  f"Serial Number: {full_manifest['device']['serial_number']}",
  f"LoRaWAN Region: {full_manifest['lorawan']['region']}",
  f"LoRaWAN Activation: {full_manifest['lorawan']['activation']}",
  f"Business Profile: {full_manifest['business_logic']['profile']}",
  f"Heartbeat ms: {full_manifest['business_logic']['heartbeat_period_ms']}",
  "",
  "Artifact Hashes (SHA-256):",
]

for artifact in firmware_artifacts:
  summary_lines.append(f"- {artifact['name']}: {artifact['sha256']}")

(docs_dir / "build_manifest_summary.txt").write_text("\n".join(summary_lines) + "\n", encoding="utf-8")
PY

# Normalize line endings to LF in generated package content.
for file in "${FLASH_DIR}/flash_stlink.sh" "${FLASH_DIR}/flash_uart.sh" "${DOCS_DIR}/FLASHING_GUIDE.txt"; do
  sed -i 's/\r$//' "${file}"
done

for file in "${DOCS_DIR}/build_manifest_summary.txt"; do
  sed -i 's/\r$//' "${file}"
done

(
  cd "${OUT_DIR}"
  rm -f "${ZIP_FILE}"
  zip -r "${ZIP_FILE}" "wio-e5-telemetry-production" >/dev/null
)

echo "[packaging] Production artifact bundle ready: ${ZIP_FILE}"
