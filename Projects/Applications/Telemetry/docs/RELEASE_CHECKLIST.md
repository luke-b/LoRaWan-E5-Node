# Telemetry Release Checklist (Business Logic Evidence)

Use this checklist before tagging or publishing a firmware release.

## Scope

This checklist verifies that release evidence covers business logic, not only successful compilation.

## Required Inputs

- Current production artifacts: `telemetry.elf`, `telemetry.hex`, `telemetry.bin`, `telemetry.map`
- Current traceability matrix: `docs/TEST_TRACEABILITY.md`
- Latest CI runs on target commit:
  - `Telemetry Emulation`
  - `Telemetry Production Build`

## 1. Requirement Traceability Gate

- [ ] `docs/TEST_TRACEABILITY.md` contains all active `REQ-*` rows.
- [ ] No `REQ-*` row contains `Pending` in test/CI evidence columns.
- [ ] `Telemetry Emulation` artifacts include `requirement_coverage.txt`.
- [ ] `requirement_coverage.txt` confirms:
  - total requirements > 0
  - pending rows = 0

## 2. Unit Evidence

- [ ] Host unit suites pass on release commit:
  - `test_telemetry_app`
  - `test_telemetry_logic`
- [ ] Unit logs are present in emulation artifacts (`unit.log`).
- [ ] Business decision tests include:
  - door pending/debounce gating
  - periodic wake gating
  - concurrent wake intent (door + periodic)

## 3. Integration/Emulation Evidence

- [ ] Renode Robot suite passes on release commit.
- [ ] Mandatory scenarios observed in UART/report output:
  - `heartbeat`
  - `door_alarm`
  - `water_alarm`
  - `counter_overflow`
  - `wake_decisions`
  - `interleaved_wakes`
- [ ] No flaky or retried test case in the final accepted run.

## 4. Production Build Evidence

- [ ] `Telemetry Production Build` workflow is green on release commit.
- [ ] Bundle artifact exists: `telemetry-production-bundle`.
- [ ] Bundle includes:
  - firmware files (`.elf/.hex/.bin/.map`)
  - flash scripts
  - flashing guide
  - build manifests:
    - `build_manifest_full.json`
    - `build_manifest_redacted.json`
    - `build_manifest_summary.txt`

## 5. LoRaWAN Provisioning and Join Evidence

- [ ] Build manifest contains expected LoRaWAN parameters for target deployment:
  - region
  - activation mode
  - unique `device_eui`
  - unique `join_eui`
  - per-device `app_key`/`nwk_key`
- [ ] Manufacturing record links device serial number to manifest `device_id`.
- [ ] Join commissioning evidence exists for at least one representative device in the batch.
- [ ] Any mismatch between planned and built parameters is resolved before release publish.

## 6. Behavioral Review (Human Sign-off)

- [ ] Message policy confirmed:
  - heartbeat is unconfirmed
  - alarms are confirmed
- [ ] Payload contract unchanged or backend decoder updated accordingly.
- [ ] Any business-rule change is reflected in:
  - `REQ-*` matrix
  - tests
  - release notes

## 7. Final Release Decision

Release can proceed only if all boxes above are checked.

Recommended release note line:

`Business logic evidence complete: unit + emulation + traceability + production artifact workflows passed on commit <sha>.`
