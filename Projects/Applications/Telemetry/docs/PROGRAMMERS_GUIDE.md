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

## Debug guidance
- Use trace outputs from adv_trace/usart_if path
- Inspect telemetry.map for symbol/link surprises
- Verify stack and memory footprints using arm-none-eabi-size and map file

## Release governance
- Keep business logic traceability in docs/TEST_TRACEABILITY.md up to date.
- Use docs/RELEASE_CHECKLIST.md before publishing any firmware release.
