# Telemetry User Guide

## What this project is
Telemetry is a low-power LoRaWAN application for Wio-E5 focused on utility pit monitoring.

Functional goals:
- Pulse counting
- Door/opening alarm
- Water leak check by ADC
- Periodic heartbeat uplink
- Deep sleep operation

## Features at a glance
- LoRaWAN stack integration
- Sensor and status payload creation
- Event-driven alarm behavior
- RTC/LPTIM based timing and counting
- STOP2 low-power loop

## Build

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make
```

Expected artifacts:
- telemetry.elf
- telemetry.hex
- telemetry.bin

## Flash
Example with STM32CubeProgrammer CLI:

```bash
STM32_Programmer_CLI -c port=SWD -w telemetry.hex -v -rst
```

## Runtime behavior
At runtime the app:
1. Initializes HAL, clocks, GPIO, RTC, LPTIM, ADC, LoRaWAN
2. Joins LoRaWAN network
3. Sleeps in STOP2
4. Wakes on interrupts/events
5. Sends alarms or heartbeat payloads

## Payload format
Current payload (7 bytes):
- Byte 0: Message type
- Bytes 1-4: Pulse counter (big endian)
- Byte 5: Battery level
- Byte 6: Status bits

Message types:
- 0x01 heartbeat
- 0x02 door alarm
- 0x03 water alarm

## Typical commissioning flow
1. Program device with telemetry.hex
2. Configure network credentials in LoRaWAN app config files
3. Power cycle device
4. Confirm join event in logs or network server
5. Trigger door/contact test and validate uplink

## Troubleshooting

### Build succeeds but no uplink
- Verify region and credentials
- Verify antenna path and RF switch control
- Verify gateway coverage and channel plan

### Frequent wakeups and power drain
- Check EXTI pin pull and debounce behavior
- Ensure periodic logic does not busy-loop
- Confirm STOP2 entry path is reached

### Bad battery readings
- Validate ADC channel config
- Validate divider/reference assumptions
- Check sampling timing and VREF dependencies

## Safety note
If deploying in real infrastructure, validate enclosure, ingress protection, and fail-safe behavior under sensor faults before production rollout.
