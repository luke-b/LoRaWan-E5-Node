# HelloWorld User Guide

## What this project is
HelloWorld is a minimal firmware for Wio-E5 (STM32WLE5xx) that sends a UART message once per second.

Message:
- Hello World z Wio-E5!

Purpose:
- Validate toolchain
- Validate board bring-up
- Validate UART wiring and terminal settings

## Quick start

### Build
From the project folder:

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/HelloWorld
make
```

Expected artifacts:
- hello_world.elf
- hello_world.hex
- hello_world.bin

### Flash
Use your preferred flashing tool.

Example with STM32CubeProgrammer CLI:

```bash
STM32_Programmer_CLI -c port=SWD -w hello_world.hex -v -rst
```

### Serial terminal settings
- Port: your USB/UART adapter port
- Baud: 9600
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

You should see one line per second.

## Wiring notes
Current project config uses:
- USART1 TX: PB6
- USART1 RX: PB7

Connect PB6 to adapter RX.
Connect PB7 to adapter TX (optional unless you add RX logic).
Connect GND to GND.

## Troubleshooting

### Build fails with arm-none-eabi command not found
Install toolchain:

```bash
sudo apt update && sudo apt install -y gcc-arm-none-eabi
```

### Build fails due to FPU flags
STM32WLE5 does not use hardware FPU in this project setup. Keep MCU flags at:
- -mcpu=cortex-m4
- -mthumb

### No UART output
Check:
- Correct baud rate (9600)
- Correct pin wiring
- Board is actually running flashed image
- Ground is shared between board and UART adapter

## Next steps
After HelloWorld works, move to the Telemetry project for low-power sensing and LoRaWAN uplink logic.
