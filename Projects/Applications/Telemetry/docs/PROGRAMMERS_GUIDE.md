# Telemetry Programmers Guide

## High-level architecture
The Telemetry app combines:
- Core HAL setup and low-power control
- LoRaWAN middleware stack
- Board support package (BSP)
- Utility support modules (timer, sequencer, trace, systime)

Key directories:
- Core/Src and Core/Inc
- LoRaWAN/App and LoRaWAN/Target
- Makefile for full dependency assembly

## Main control flow
Primary execution is in Core/Src/main.c:
1. HAL and clock init
2. Peripheral init (GPIO/RTC/LPTIM/ADC)
3. LoRaWAN init and join
4. Event processing loop
5. STOP2 entry and wake restore

## Event model
Flags are set by interrupts/callbacks and consumed in main loop:
- Door alarm flag
- RTC wakeup flag

This keeps interrupt handlers small and deterministic.

## LoRaWAN integration details
- LmHandler is used for uplinks
- Confirmed messages for alarms
- Unconfirmed messages for heartbeat
- Radio and region logic resolved via middleware and target files

## Dependency map
Telemetry depends on these major source groups:
- Core: main/system/msp/sys_app/adc_if/timer_if/sys_debug/sys_sensors/subghz/dma/usart/usart_if
- LoRaWAN Mac, LmHandler, selected packages
- SubGHz radio driver
- HAL modules including subghz/lptim/rtc/adc/uart/dma
- Utilities: lpm, mem, systime, tiny_vsnprintf, seq, timer, adv_trace

## Why selected package subset is used
The Makefile intentionally includes specific LmHandler package files instead of wildcarding all package sources.
Reason:
- Some optional FUOTA package units require additional integration headers not included in this app profile.

## Extension points

### Add a new sensor to payload
1. Add read function in Core
2. Extend payload in SendLoraMessage
3. Update backend decoder accordingly

### Change heartbeat interval
Adjust RTC wake scheduling logic in main loop and timer setup.

### Add downlink command handling
Implement parse logic in LoRaWAN application receive callbacks.

## Test architecture

Telemetry quality gates are split into two lanes:
- hardware firmware build for the real Wio-E5 image
- emulation firmware build that reuses the same Telemetry payload/state logic but replaces unsupported STM32WL RF dependencies with a minimal Cortex-M UART harness for Renode

The shared logic lives in Core/Src/telemetry_app.c and is the main pre-HIL verification target.

Emulation assets:
- Emulation/renode/wio_e5_telemetry_emulation.repl
- Emulation/tests/integration_tests.robot
- Emulation/scripts/run_ci.sh
- Dockerfile.emulation
- .github/workflows/telemetry-emulation.yml

Why the split is necessary:
- Renode currently does not provide an official STM32WL/Wio-E5 machine with usable SubGHz LoRa support
- pretending otherwise would create a false sense of coverage
- the current setup validates deterministic application behavior in CI and leaves RF acceptance to a later HIL stage

## Performance and power notes
- Keep ISR work tiny and move logic to loop
- Use deferred processing flags
- Avoid unnecessary wakeups
- Keep ADC and sensor power rails enabled only for measurement windows

## Build system notes
This Makefile is intentionally explicit about include/source coverage to avoid hidden IDE-generated dependencies.

When adding files, update:
- INC_DIRS
- SRC_FILES

### Hardware vs Emulation builds
The Makefile supports two build profiles via `BUILD_PROFILE` variable:
- `BUILD_PROFILE=hardware` (default): Full HAL, LoRaWAN, SubGHz stack for Wio-E5  
  Command: `make` or `make BUILD_PROFILE=hardware`
- `BUILD_PROFILE=emulation`: Minimal Cortex-M + UART for Renode CI  
  Command: `make BUILD_PROFILE=emulation`

Shared logic resides in Core/Src/telemetry_app.c and is compiled into both profiles.

## Running CI Locally

### Prerequisites for local emulation tests
- Linux/Mac with Bash
- arm-none-eabi-gcc toolchain >= 14.x
- Renode (latest portable build)
- Robot Framework (Python 3)

### Installation for Ubuntu/Debian
```bash
# Install ARM toolchain
wget https://developer.arm.com/downloads/view/ARM-GNU-Toolchain-Releases
# Extract to /opt/toolchains

# Install Renode
wget https://builds.renode.io/renode-latest.linux-portable.tar.gz
# Extract to /opt/renode
export PATH="/opt/renode:$PATH"

# Install Robot Framework
pip3 install --user robotframework

# Update PATH
export PATH="/opt/toolchains/arm-gnu-toolchain-XX/bin:$PATH"
```

### Local test execution
```bash
cd Projects/Applications/Telemetry

# Run just the emulation build
make BUILD_PROFILE=emulation clean all

# Run full CI pipeline (build + Renode tests + artifacts)
bash Emulation/scripts/run_ci.sh

# Check results
ls -la emulation-artifacts/
cat emulation-artifacts/uart.log
# Open emulation-artifacts/report.html in browser
```

### Emulation test scenarios
The emulation harness runs 4 deterministic test scenarios via Robot Framework:

1. **Heartbeat Message**: Message type 0x01, counter=0, assertions check payload structure
2. **Door Alarm**: Message type 0x02, counter=5, door flag (0x01) set, confirmed=true
3. **Water Alarm**: Message type 0x03, counter=9, water flag (0x02) set
4. **Counter Overflow**: LPTIM 16-bit wraparound (65534→3) correctly detected, total pulses=5

Payload format (7 bytes):
```
Byte 0: Message type (0x01/0x02/0x03)
Bytes 1-4: Water pulse counter (big-endian 32-bit)
Byte 5: Battery level (0-255, here 0x98-0xC8)
Byte 6: Status flags (0x00, 0x01 for door, 0x02 for water)
```

### CI script flow
Emulation/scripts/run_ci.sh performs:
1. Toolchain verification (arm-none-eabi-gcc, renode-test)
2. Clean emulation build with detailed logging to emulation-artifacts/build.log
3. ELF size validation and verification
4. Renode test execution via Robot Framework (output to emulation-artifacts/uart.log)
5. Report collection (report.html, log.html)
6. Final summary highlighting PASSED/FAILED status

## GitHub Actions CI

### Workflow trigger
`.github/workflows/telemetry-emulation.yml` runs on:
- Push to Telemetry path
- Pull request touching Telemetry path
- Manual workflow dispatch (GitHub Actions UI)

### Workflow steps
1. **Checkout**: Clone repo
2. **Docker Build**: Build telemetry-emulation-ci image from Dockerfile.emulation
3. **Docker Run**: Execute run_ci.sh inside container
4. **Verify Artifacts**: Confirm emulation-artifacts/ exists and has expected files
5. **Upload Artifacts**: Package reports and telemetry_emulation.elf for download (30-day retention)
6. **Summary**: Generate GitHub step summary showing PASSED/FAILED and artifact list

### Accessing CI results
After workflow completes:
1. Check workflow logs in Actions tab
2. Download artifacts from workflow run:
   - `telemetry-emulation-reports` contains:
     - build.log (compiler output)
     - uart.log (Renode/Robot output)
     - report.html (Robot Framework HTML report)
     - log.html (Robot Framework detailed log)
     - telemetry_emulation.elf (compiled emulation firmware)

### Docker image details
Dockerfile.emulation creates an Ubuntu 24.04 container with:
- ARM GNU Toolchain 14.2.rel1 installed and in PATH
- Renode latest portable build installed and in PATH
- Robot Framework installed via pip3
- Workdir: /workspace (repo mounted here during run)

## Debug guidance
- Use trace outputs from adv_trace/usart_if path
- Inspect telemetry.map for symbol/link surprises
- Verify stack and memory footprints using arm-none-eabi-size and map file

## Troubleshooting

### Emulation build fails with "telemetry_emulation_log.h: No such file"
**Cause**: INC_DIRS in Makefile doesn't include Emulation/Inc for emulation profile
**Fix**: Verify Makefile has `Emulation/Inc` in INC_DIRS before hardware-specific includes

### Robot tests hang (timeout)
**Cause**: Renode platform file not found, or ELF not at expected path
**Fix**: Verify paths in integration_tests.robot match actual file locations:
```robot
${REPL}         ${CURDIR}/../renode/wio_e5_telemetry_emulation.repl
${ELF}          ${CURDIR}/../../telemetry_emulation.elf
```

### Docker build fails with "cannot find toolchain"
**Cause**: Network issue downloading ARM toolchain or Renode
**Fix**: Check internet connection, verify URLs in Dockerfile.emulation are current

### "PROJECT EXECUTION FAILED" in emulation output
**Cause**: One or more assertion failed in main_emulation.c test scenarios
**Fix**: Check uart.log for specific ASSERT FAIL lines, verify telemetry_app.c payload builder logic

### GitHub Actions workflow doesn't trigger
**Cause**: Workflow path filter doesn't match commit changes
**Fix**: Ensure push touches Projects/Applications/Telemetry/** or .github/workflows/telemetry-emulation.yml
