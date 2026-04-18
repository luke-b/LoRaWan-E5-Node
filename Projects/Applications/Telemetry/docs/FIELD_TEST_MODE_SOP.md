# Field Test Mode SOP - Telemetry Application

**Version:** 1.0  
**Date:** 2026-04-18  
**Audience:** IoT Engineers (field operations)

---

## Purpose

Enable IoT Engineers to verify device-to-network connectivity in the field **without external tools**. By pressing the device's button for 2+ seconds after deployment, the device sends a minimal test message to the ČRA LoRaWAN network. The engineer can immediately verify message reception in the CRA IoT portal, confirming:

- Device is successfully joined to ČRA
- Uplink connectivity is operational
- Device ID is correctly provisioned
- LoRaWAN parameters are valid for the target region

This procedure is a **quick sanity check** following factory flashing and CRA provisioning steps.

---

## Inputs

1. **Deployed Seeed Wio-E5 device** with Telemetry application firmware (post-flash)
2. **Device status: Joined to CRA network** (indicated by steady green LED, not blinking)
3. **ČRA IoT Portal** access credentials (URL: iot-portal.cra.network or equivalent)
4. **Expected Device ID** from provisioning documentation (4-byte hex, e.g., `0x12345678`)

---

## Prerequisites

- ✅ Device has been flashed with production Telemetry firmware (see [FACTORY_USB_C_FLASH_SOP.md](FACTORY_USB_C_FLASH_SOP.md))
- ✅ Device has been provisioned with unique LoRaWAN parameters (DevEUI, JoinEUI, AppKey, NwkKey)
- ✅ Device has completed LoRaWAN join to ČRA network (see [CRA_LORAWAN_JOIN_SOP.md](CRA_LORAWAN_JOIN_SOP.md))
- ✅ Device is powered (battery or USB-C)
- ✅ LED indicator is **steady green** (confirmed join; not blinking yellow = joining)

---

## Procedure

### Step 1: Observe LED Status (Pre-Check)

Verify that the device LED is **steady green** (not blinking). If LED is **blinking yellow/rapid**, the device is still joining the network.

- **Action**: Wait until LED turns **steady green** (may take 30-60 seconds). If LED does not stabilize within 5 minutes, troubleshoot join process (see [CRA_LORAWAN_JOIN_SOP.md](CRA_LORAWAN_JOIN_SOP.md)).

### Step 2: Initiate Test Mode

Press and hold the device's **user button** (SW1, located on the board near the antenna) for **2+ seconds**. Count silently: `"1-one-thousand, 2-one-thousand"`.

- **Correct action**: Steady pressure; do not release prematurely.
- **Release point**: After `2-one-thousand` count (approximately 2.1 seconds). Button will not respond to shorter presses.

### Step 3: Observe LED Feedback Pattern

Within 100 milliseconds of button release, observe the LED:

- **Expected pattern**: LED blinks **5 rapid times** (ON/OFF, 100ms each, total ~1 second duration)
- **Meaning**: Device is sending test message to ČRA network
- **Failed pattern**: No LED response → Button press may have been too short or device not ready. Retry from Step 2.

### Step 4: Access CRA IoT Portal

Open the ČRA IoT Portal in browser:

- **URL**: `https://iot-portal.cra.network` (or your regional endpoint)
- **Authentication**: Use your IoT team credentials
- **Navigation**: Go to **Devices** → **Select Target Device** (search by name or Device ID)

### Step 5: Verify Message Receipt

In the device's **Message History** or **Activity Log** section:

1. **Time check**: Verify a new uplink message appears with timestamp **within ±5 seconds of button press** (accounts for clock skew).
2. **Payload check**: Expand message details; verify:
   - **Port**: 1 (standard telemetry port)
   - **Payload (hex)**: Starts with `04` (test message type), followed by 4 bytes of device ID
   - Example: `04 12 34 56 78` (message type `04` + device ID `0x12345678`)
3. **Signal quality**: Confirm RSSI (signal strength) and SNR (signal-to-noise ratio) are recorded
   - Typical range: RSSI between -120 to -70 dBm (outdoor/indoor varies)
   - SNR: positive values preferred (>0 dB)

### Step 6: Capture Evidence

Screenshot or photograph the CRA portal message for documentation:

- **Include in screenshot**: Device name, timestamp, payload (hex), signal metrics (RSSI/SNR)
- **Store**: Project documentation or field logbook with device serial number and test date

---

## Verification Checklist

Before concluding the test, verify all items:

| Item | Status | Notes |
|------|--------|-------|
| LED blinks 5 times after button release | ✓ / ✗ | If ✗, retry button press |
| Message appears in CRA portal within 30 sec | ✓ / ✗ | If ✗, check join status |
| Timestamp matches press ±5 sec | ✓ / ✗ | Account for clock skew |
| Payload hex starts with `04` | ✓ / ✗ | Message type correct |
| Device ID bytes match provisioning doc | ✓ / ✗ | Confirms right device tested |
| RSSI/SNR recorded | ✓ / ✗ | Signal quality acceptable? |
| Screenshot captured for records | ✓ / ✗ | Archive in project docs |

---

## Troubleshooting

### Problem: No LED Blink After Button Press

**Likely Cause**: Button press was too short (<2 seconds) or device not ready.

**Solution**:
1. Verify LED is steady green (device is joined). If not, wait for join to complete.
2. Count aloud: "1-one-thousand, 2-one-thousand" before releasing button.
3. Retry testing.

---

### Problem: LED Blinks, But No Message in Portal

**Likely Cause**: Device joined LoRaWAN network, but uplink path to CRA backend is blocked (e.g., port filtering, duty-cycle limit).

**Solution**:
1. Check CRA portal **device settings** → **Port filters**: Ensure port 1 is **not blocked**.
2. Wait 2+ minutes; LoRaWAN duty cycle may delay message.
3. Check device uplink counter in portal: If counter did not increment, uplink failed.
4. Verify **device status** in portal (should say "Joined" or "Activated", not "Pending Join").
5. If still no message, re-run [CRA_LORAWAN_JOIN_SOP.md](CRA_LORAWAN_JOIN_SOP.md) to re-join network.

---

### Problem: Message Appears, But Device ID Doesn't Match Provisioning

**Likely Cause**: Wrong device tested, or device ID provisioned incorrectly during factory flash.

**Solution**:
1. Verify you tested the **correct device** (check physical label/serial number).
2. Cross-reference expected Device ID from flash provisioning document.
3. If Device ID is wrong, device may have been flashed with default/wrong provisioning. Re-run factory flash with correct provisioning.

---

### Problem: RSSI/SNR Values Are Poor (RSSI < -120 dBm)

**Likely Cause**: Device is far from ČRA base station or signal is obstructed (indoors, metal structure).

**Solution**:
1. Move device closer to window or outdoor area and retry test.
2. Check site survey for ČRA coverage at device location.
3. If coverage is confirmed but signal is still weak, investigate antenna connection (loose SMA connector, damaged antenna).

---

### Problem: Message Appears Multiple Times (Duplicate Receipts)

**Likely Cause**: Normal retry behavior (device may re-transmit if join not confirmed).

**Solution**:
1. This is expected behavior and not a failure. Use the **latest timestamp** message as the "official" test result.
2. If duplicates occur frequently, verify device has stable power supply and LoRaWAN MAC parameters are correct.

---

## Evidence Required

Store the following for each field test:

1. **CRA Portal Screenshot**
   - Device name, message timestamp, payload (hex), RSSI/SNR
   
2. **Test Log Entry**
   - Device serial number
   - Test date/time
   - Provisioned Device ID (expected)
   - Device ID (observed in payload)
   - RSSI value
   - Test result: **PASS** / **FAIL**
   
3. **Photos (Optional)**
   - Physical device location (building, coordinate if outdoor)
   - Signal quality evidence (if investigating coverage)

---

## Acceptance Criteria

Test is **PASS** if:

✅ Device LED blinks 5 times after 2+ sec button hold  
✅ Message appears in CRA portal within 30 seconds  
✅ Timestamp is within ±5 seconds of button press  
✅ Payload format is correct (type 0x04 + device ID)  
✅ Device ID matches provisioning documentation  
✅ RSSI and SNR are recorded (acceptable signal range)  

Test is **FAIL** if any of the above are not met. Re-run troubleshooting or escalate to firmware/provisioning team.

---

## Related Documents

- [FACTORY_USB_C_FLASH_SOP.md](FACTORY_USB_C_FLASH_SOP.md) — USB-C firmware flashing procedure
- [CRA_LORAWAN_JOIN_SOP.md](CRA_LORAWAN_JOIN_SOP.md) — LoRaWAN join to CRA network
- [IOT_ENGINEERING_LIFECYCLE.md](IOT_ENGINEERING_LIFECYCLE.md) — Full device lifecycle overview
- [README.md](../README.md) — Project documentation index

---

## Notes

- Test message is **unconfirmed** (no ACK expected from CRA backend). Message may be sent once; retries are device firmware controlled.
- Test payload is minimal (5 bytes) to reduce airtime and avoid duty-cycle issues.
- Test message does **not** include sensor readings (temperature, pressure, etc.); it is for connectivity check only.
- LED feedback pattern uses asynchronous timers; no blocking sleep during blink sequence.
- Button press duration tolerance is ±100ms; devices with slower debounce (>300ms) may require timer adjustment.

---

## Support & Escalation

If test fails repeatedly:

1. Check device [FACTORY_USB_C_FLASH_SOP.md](FACTORY_USB_C_FLASH_SOP.md) was executed correctly (firmware version visible in serial log).
2. Verify [CRA_LORAWAN_JOIN_SOP.md](CRA_LORAWAN_JOIN_SOP.md) resulted in confirmed join (join accept message in portal).
3. Review [IOT_ENGINEERING_LIFECYCLE.md](IOT_ENGINEERING_LIFECYCLE.md) phase 4 (Join Commissioning) for diagnostic flows.
4. Contact firmware team with:
   - Device serial number
   - Provisioning file (device_provisioning.env excerpt, keys redacted)
   - CRA portal logs (uplink counter, join history)
   - Button press repeatability (does it work on another device or same device multiple times?)

---

**Document Version:** v1.0 | **Last Updated:** 2026-04-18
