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

### Emulation build command
```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make BUILD_PROFILE=emulation
```

### Build outputs
- telemetry.elf
- telemetry.hex
- telemetry.bin
- telemetry.map

Emulation profile outputs:
- telemetry_emulation.elf
- telemetry_emulation.hex
- telemetry_emulation.bin
- telemetry_emulation.map

## Emulation and CI

Telemetry now includes a Renode-oriented emulation profile under Emulation/ that validates the reusable application logic before deploying to a physical Wio-E5.

What is covered in emulation:
- payload layout and message typing
- pulse counter accumulation including 16-bit overflow handling
- heartbeat, door alarm and water alarm serialization
- deterministic boot and report generation in CI

What is intentionally not claimed in emulation:
- SubGHz radio behavior
- LoRaWAN over-the-air join or coverage
- current consumption or STOP2 measurements

Run locally when Renode is installed:

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make BUILD_PROFILE=emulation clean all
renode-test Emulation/tests/integration_tests.robot
```

Run via Docker CI (works on any system with Docker):

```bash
cd /workspaces/LoRaWan-E5-Node
docker build -t telemetry-ci -f Projects/Applications/Telemetry/Dockerfile.emulation .
docker run --rm -v "$PWD:/workspace" -w /workspace telemetry-ci \
  bash Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
```

Run the complete CI pipeline (builds image, runs tests, collects artifacts):

```bash
bash Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
```

The CI script will:
1. Verify toolchain (arm-none-eabi-gcc, renode-test)
2. Build emulation firmware (BUILD_PROFILE=emulation)
3. Execute Robot Framework tests in Renode
4. Collect reports (build.log, uart.log, report.html, log.html)
5. Generate summary showing PASSED/FAILED status

Results are saved in Projects/Applications/Telemetry/emulation-artifacts/

## GitHub Actions

This repository includes `.github/workflows/telemetry-emulation.yml` which automatically runs the emulation CI on every push and PR to the Telemetry path.

View results in:
- GitHub Actions tab → Telemetry Emulation workflow
- Artifacts: telemetry-emulation-reports (build logs, test reports, emulation ELF)

For more details on CI setup, local testing, and troubleshooting, see [docs/PROGRAMMERS_GUIDE.md](docs/PROGRAMMERS_GUIDE.md#running-ci-locally).

## Next Steps for Developers

1. **Review the test architecture**: See docs/PROGRAMMERS_GUIDE.md § Test architecture
2. **Run emulation locally** (optional): Follow the local test instructions above
3. **Deploy hardware**: Flash telemetry.elf to Wio-E5 and configure LoRaWAN join parameters
4. **Validate on network**: Monitor OTAA join and uplink transmission via gateway

## Limitations

- **Emulation**: Payload serialization only. RF, power consumption, and network behavior require hardware testing.
- **Hardware testing**: Requires LoRaWAN gateway and network coverage in the deployment area.
- **Current design constraints**: Single threaded event loop; no concurrent downlink handling.

```

Or run the exact CI flow through Docker:

```bash
cd /workspaces/LoRaWan-E5-Node
docker build -t telemetry-emulation-ci -f Projects/Applications/Telemetry/Dockerfile.emulation .
docker run --rm -v "$PWD:/workspace" -w /workspace telemetry-emulation-ci \
  bash Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
```

Artifacts expected from renode-test:
- report.html
- log.html
- uart.log (CI transcript with UART-visible scenario output)

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
