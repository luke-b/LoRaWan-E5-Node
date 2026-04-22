# Telemetry for Wio-E5

Language: English | [Cesky](README.cs.md)

A production-style embedded app for monitoring a utility pit using low power, events, and LoRaWAN uplinks.

Think of it as: always listening, usually sleeping, occasionally speaking.

## TL;DR
- Platform: Wio-E5 (STM32WLE5xx)
- Domain: pit telemetry (pulse counting, door alarm, leak check)
- Network: LoRaWAN
- Power model: deep sleep first (STOP2)
- Build system: Make + arm-none-eabi-gcc
- Artifacts: telemetry.elf, telemetry.hex, telemetry.bin

## Functional Specification

### Core functions
1. Asynchronous pulse counting using LPTIM1
2. Door-open alarm on EXTI trigger
3. Water leak check through ADC with switched excitation
4. Periodic heartbeat payload uplink
5. Low-power operation using STOP2 wake/sleep cycle

### Behavioral model
- Event-driven main loop with flag-based state transitions
- Interrupts are lightweight and defer work to loop logic
- LoRaWAN messages are sent by priority:
  - alarms first
  - heartbeat periodically

### Payload contract (current app)
- Byte 0: message type
- Byte 1-4: cumulative pulse counter (big-endian)
- Byte 5: battery level
- Byte 6: status bitfield

Message types:
- 0x01 heartbeat
- 0x02 door alarm
- 0x03 water alarm

## Architecture Orientation for Newcomers

### Big picture
Telemetry combines five layers:
1. Application logic in Core/Src/main.c
2. Peripheral support units (timer_if, adc_if, sys_app, usart_if, subghz, etc.)
3. LoRaWAN application glue in LoRaWAN/App
4. LoRaWAN middleware + SubGHz radio stack in Middlewares
5. HAL/CMSIS/BSP foundation in Drivers

### Where to read first
1. Core/Src/main.c for top-level behavior
2. LoRaWAN/App/lora_app.c for join/send handling
3. Makefile to understand dependency assembly
4. docs/PROGRAMMERS_GUIDE.md for deeper internals

## Build Information

### Prerequisites
- arm-none-eabi-gcc toolchain
- make

### Build command
```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make
```

## Build with device-specific LoRaWAN manifest (IoT engineering flow)

Use this flow when preparing a production bundle for a specific new device.

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry

export TELEMETRY_DEVICE_ID="PIT-DEVICE-0001"
export TELEMETRY_SERIAL_NUMBER="SN-0001"
export TELEMETRY_BUSINESS_PROFILE="czech-field-default"

export TELEMETRY_LORAWAN_REGION="LORAMAC_REGION_EU868"
export TELEMETRY_LORAWAN_ACTIVATION="ACTIVATION_TYPE_OTAA"
export TELEMETRY_LORAWAN_DEVICE_EUI="0080E10123456789"
export TELEMETRY_LORAWAN_JOIN_EUI="70B3D57ED005ABCD"
export TELEMETRY_LORAWAN_APP_KEY="00112233445566778899AABBCCDDEEFF"
export TELEMETRY_LORAWAN_NWK_KEY="00112233445566778899AABBCCDDEEFF"

make clean all
./scripts/package_production_artifacts.sh
```

Generated bundle now includes:
- `docs/build_manifest_full.json` (internal manifest with full keys)
- `docs/build_manifest_redacted.json` (masked values for sharing)
- `docs/build_manifest_summary.txt` (human-readable checksum summary)

Validation and enforcement flags:
- `TELEMETRY_ENFORCE_DEVICE_METADATA=1` (default): fail packaging if `TELEMETRY_DEVICE_ID` or `TELEMETRY_SERIAL_NUMBER` is missing.
- `TELEMETRY_ENFORCE_CUSTOM_KEYS=1`: fail packaging if LoRaWAN values still match defaults from source headers.

Production CI applies `TELEMETRY_LORAWAN_*` values to `LoRaWAN/App/se-identity.h` before compiling firmware. This keeps the generated firmware, `build_manifest_full.json`, and CRA provisioning values aligned. If `TELEMETRY_LORAWAN_NWK_KEY` is omitted, the build-time identity step uses the same value as `TELEMETRY_LORAWAN_APP_KEY`, which is the expected setup for LoRaWAN 1.0.x deployments that expose a single OTAA AppKey.

Provisioning helper template:
- `scripts/templates/device_provisioning.env.template`

If LoRaWAN values are not set through environment variables, packaging falls back to values found in source headers.

### Build outputs
- telemetry.elf
- telemetry.hex
- telemetry.bin
- telemetry.map

## Flash to Wio-E5 from Windows via USB-C

The procedures below are detailed for newcomers.

### Option A (recommended for USB-C only): UART bootloader with STM32CubeProgrammer

#### 1. Install software on Windows
- STM32CubeProgrammer
- USB-UART driver for your board bridge chip (CP210x/CH340 if required)

#### 2. Put board in STM32 ROM bootloader mode
Typical sequence:
1. Hold BOOT button (or set BOOT0)
2. Tap RESET
3. Release BOOT

#### 3. Identify serial port
In Device Manager, note COMx assigned to Wio-E5.

#### 4. Flash in STM32CubeProgrammer
1. Start STM32CubeProgrammer
2. Select UART
3. Port = COMx
4. Baud = 115200 (or supported)
5. Connect
6. Select telemetry.hex
7. Download/Program
8. Return board to normal boot and reset

#### 5. Validate quickly
- Device joins LoRaWAN network
- Trigger door event and observe alarm uplink
- Verify periodic heartbeat behavior

### Option B: ST-LINK (if your development kit provides it)
1. Connect USB-C
2. Open STM32CubeProgrammer
3. Select ST-LINK
4. Connect and flash telemetry.hex
5. Reset and observe runtime

## Newcomer Bring-up Checklist
- Build succeeds
- Flash succeeds
- Board boots cleanly
- LoRaWAN join succeeds
- Alarm and heartbeat paths both verified
- Sleep/wake cycle behaves as expected

## Troubleshooting

### Build succeeds but no uplinks
- Wrong LoRaWAN credentials/region
- Gateway not reachable
- Antenna or RF path issue

### Too many wakeups, battery drains fast
- Noisy EXTI input
- Debounce/pull config mismatch
- Logic path preventing STOP2 residency

### CubeProgrammer UART cannot connect
- Not in bootloader mode
- Wrong COM or baud
- Missing USB-UART driver

## Safety and deployment note
Before field deployment, run environmental testing for moisture, power stability, and sensor fault handling.

## Related docs
- docs/USER_GUIDE.md
- docs/PROGRAMMERS_GUIDE.md
- docs/COOKBOOK.md
- docs/TEST_TRACEABILITY.md
- docs/RELEASE_CHECKLIST.md
- docs/IOT_ENGINEERING_LIFECYCLE.md
- docs/FACTORY_USB_C_FLASH_SOP.md
- docs/CRA_LORAWAN_JOIN_SOP.md
- docs/FIELD_TEST_MODE_SOP.md
