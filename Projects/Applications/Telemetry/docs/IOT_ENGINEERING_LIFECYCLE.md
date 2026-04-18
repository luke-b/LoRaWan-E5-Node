# IoT Engineering Lifecycle: Wio-E5 Telemetry

This runbook defines the practical end-to-end lifecycle for a new field device.

Scope:
- Factory board with default AT firmware
- Flash to Telemetry application firmware over USB-C
- LoRaWAN provisioning for CRA network join
- Build and deployment evidence through generated manifests
- Service, rollback, and device decommission procedures

## 1. Roles and Inputs

Required role:
- IoT Engineer with access to firmware build environment and device credentials

Required inputs per device:
- `device_id` (internal inventory identifier)
- `serial_number` (physical device serial)
- LoRaWAN unique values:
  - `device_eui` (16 hex chars)
  - `join_eui` (16 hex chars)
  - `app_key` (32 hex chars)
  - `nwk_key` (32 hex chars)
- Deployment profile:
  - `region` (for CRA use `LORAMAC_REGION_EU868`)
  - `activation` (default `ACTIVATION_TYPE_OTAA`)
  - business profile name

## 2. Factory Intake (AT Firmware State)

1. Connect Wio-E5 over USB-C.
2. Verify serial enumeration on host.
3. Confirm board is currently factory AT firmware (expected for new board).
4. Record intake in install log with `device_id` and `serial_number`.

## 3. Build with Device Parameters and Manifest

1. Open firmware workspace:
   - `cd Projects/Applications/Telemetry`
2. Export per-device values:

```bash
export TELEMETRY_DEVICE_ID="PIT-DEVICE-0001"
export TELEMETRY_SERIAL_NUMBER="SN-0001"
export TELEMETRY_BUSINESS_PROFILE="czech-field-default"

export TELEMETRY_LORAWAN_REGION="LORAMAC_REGION_EU868"
export TELEMETRY_LORAWAN_ACTIVATION="ACTIVATION_TYPE_OTAA"
export TELEMETRY_LORAWAN_DEVICE_EUI="0080E10123456789"
export TELEMETRY_LORAWAN_JOIN_EUI="70B3D57ED005ABCD"
export TELEMETRY_LORAWAN_APP_KEY="00112233445566778899AABBCCDDEEFF"
export TELEMETRY_LORAWAN_NWK_KEY="00112233445566778899AABBCCDDEEFF"
```

3. Build and package:

```bash
make clean all
./scripts/package_production_artifacts.sh
```

4. Confirm generated bundle exists:
- `production-artifacts/wio-e5-telemetry-production-bundle.zip`

## 4. Manifest Outputs (Mandatory Evidence)

The production package includes:
- `docs/build_manifest_full.json`
- `docs/build_manifest_redacted.json`
- `docs/build_manifest_summary.txt`

Manifest content requirements:
- Build identity:
  - git SHA, branch/ref, profile, CI run ID, toolchain
- Device identity:
  - `device_id`, `serial_number`
- LoRaWAN provisioning values:
  - region, activation, EUI values, keys, app port, ADR state, data rate
- Business logic values:
  - profile, heartbeat period, payload structure identifiers
- Artifact integrity:
  - SHA-256 for `telemetry.bin/hex/elf/map`

Policy:
- `build_manifest_full.json`: internal handling only.
- `build_manifest_redacted.json`: shareable version for broad teams.

## 5. Flash Procedure (USB-C, UART Bootloader)

1. Put board into STM32 ROM boot mode.
2. Use package script:
   - Linux/macOS: `flash/flash_uart.sh`
   - Windows: `flash/flash_uart_windows.bat`
3. Flash `firmware/telemetry.hex`.
4. Reset into normal boot mode.

Expected outcome:
- Device no longer presents AT command runtime behavior.
- Telemetry application boot behavior is visible.

## 6. CRA LoRaWAN Join Commissioning

1. Register device in CRA backend using the same unique values from manifest.
2. Ensure region and profile match build manifest.
3. Power cycle/reset device and observe join flow.
4. Record join evidence:
- time
- device_id/serial
- join accepted (yes/no)
- operator initials

If join fails:
- verify key/EUI mismatch first
- verify region mismatch second
- verify RF gateway coverage third

## 7. Deployment Handover

Before field installation, archive:
- production bundle zip
- full manifest (internal store)
- redacted manifest (project docs/release evidence)
- join commissioning evidence

## 8. Service Lifecycle

Service scenarios:
1. Same-version reflash:
- use existing approved bundle + same manifest
2. Version upgrade:
- regenerate bundle + new manifest
- repeat commissioning evidence
3. Emergency rollback:
- flash previous approved bundle
- log rollback reason and timestamp

## 9. Decommission Lifecycle

When retiring a device:
1. Mark `device_id` as decommissioned.
2. Invalidate LoRaWAN identity in network backend.
3. Archive final service/decommission record with manifest reference.
4. Ensure no further joins/uplinks are accepted for that identity.

## 10. Acceptance Criteria

Lifecycle is compliant only when all are true:
- Device-specific LoRaWAN values were defined before packaging.
- Production bundle includes both full and redacted manifests.
- Flash was executed from approved production bundle.
- CRA join succeeded with the same manifest values.
- Service/decommission actions remain traceable to manifest and build SHA.
