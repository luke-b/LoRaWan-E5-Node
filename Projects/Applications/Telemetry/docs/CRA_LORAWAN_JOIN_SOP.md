# CRA LoRaWAN Join SOP (Telemetry)

This SOP defines commissioning steps for successful OTAA join of a newly flashed Wio-E5 node in CRA LoRaWAN network.

## 1. Purpose

Validate that each newly provisioned device can join CRA network with its unique LoRaWAN parameters.

## 2. Required Inputs

From `build_manifest_full.json`:
- `lorawan.device_eui`
- `lorawan.join_eui`
- `lorawan.app_key`
- `lorawan.nwk_key`
- `lorawan.region`
- `lorawan.activation`

Expected defaults for this project:
- region: `LORAMAC_REGION_EU868`
- activation: `ACTIVATION_TYPE_OTAA`

## 3. CRA Provisioning Steps

1. Create/register device in CRA backend.
2. Enter DevEUI/JoinEUI/AppKey/NwkKey exactly as in manifest.
3. Verify no duplicate DevEUI exists.
4. Confirm EU868 regional profile.

## 4. Device Join Procedure

1. Power cycle the flashed device.
2. Observe startup and join attempt via UART/service logs.
3. Confirm join acceptance in CRA backend.
4. Record commissioning evidence.

## 5. Evidence to Archive

- device identity (`device_id`, `serial_number`)
- build SHA and manifest identifier
- join status (`accepted`/`failed`)
- timestamp and operator
- failure reason (if applicable)

## 6. Troubleshooting Decision Flow

If join fails:
1. Check mismatch in DevEUI/JoinEUI/AppKey/NwkKey.
2. Check region mismatch (must align with deployment plan).
3. Check gateway/RF coverage.
4. Check device time and reset sequence.
5. Re-commission once after correcting parameters.

## 7. Acceptance Criteria

Commissioning is accepted only when:
- Join accepted by CRA backend.
- Evidence is linked to manifest and firmware commit SHA.
- Device enters normal telemetry operation after join.
