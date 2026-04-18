# Factory USB-C Flash SOP (Wio-E5)

This SOP covers replacement of default factory AT firmware with Telemetry application firmware over USB-C.

## 1. Purpose

Ensure repeatable factory-stage flashing with traceable evidence and correct build-to-device mapping.

## 2. Inputs

- Approved production bundle: `wio-e5-telemetry-production-bundle.zip`
- Device identifiers:
  - `device_id`
  - `serial_number`
- Build manifest:
  - `build_manifest_full.json` (internal)
  - `build_manifest_redacted.json` (shareable)

## 3. Prerequisites

- STM32CubeProgrammer installed
- USB-C cable with data support
- Access rights to device provisioning values

## 4. Procedure

1. Connect device by USB-C.
2. Confirm serial/COM interface appears.
3. Put board into STM32 ROM bootloader mode.
4. Open production bundle and use:
   - `flash/flash_uart.sh` (Linux/macOS), or
   - `flash/flash_uart_windows.bat` (Windows).
5. Program `firmware/telemetry.hex`.
6. Return board to normal boot mode and reset.

## 5. Immediate Verification

- Device boots Telemetry firmware (not AT shell behavior).
- UART log indicates application startup.
- Manifest `device_id` and `serial_number` match installation record.

## 6. Mandatory Evidence

Store per-device:
- bundle version and commit SHA
- flashing timestamp
- operator name
- pass/fail result
- manifest reference

## 7. Failure Handling

If flash fails:
1. Verify bootloader mode.
2. Verify COM/TTY selection.
3. Verify cable/driver integrity.
4. Retry once.
5. If still failing, quarantine device and log incident.
