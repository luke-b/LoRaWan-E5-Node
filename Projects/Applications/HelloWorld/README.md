# HelloWorld for Wio-E5

Language: English | [Cesky](README.cs.md)

A tiny, friendly, no-magic firmware that proves your board, toolchain, and serial path are alive.

If this app works, your project foundation is healthy.

## TL;DR
- MCU: STM32WLE5xx (Wio-E5)
- Behavior: sends a UART line every second
- Output message: Hello World z Wio-E5!
- Build system: Make + arm-none-eabi-gcc
- Artifacts: hello_world.elf, hello_world.hex, hello_world.bin

## Functional Specification

### Goals
- Verify startup, clocks, and HAL init
- Verify USART1 pin config and electrical wiring
- Provide predictable serial heartbeat for bring-up

### Runtime behavior
1. Boot and initialize HAL/system clock
2. Initialize USART1
3. Enter infinite loop
4. Send one line over UART
5. Delay ~1000 ms
6. Repeat forever

### Interfaces
- UART TX: PB6
- UART RX: PB7
- Default baud: 9600 8N1

## Project Layout
- Core/Inc: headers
- Core/Src: firmware sources
- Makefile: build entrypoint
- STM32WLE5JCIX_FLASH.ld: linker script
- startup_stm32wle5jcix.s: startup assembly
- docs/: deeper docs for users and developers

## Build Information

### Prerequisites
- arm-none-eabi-gcc toolchain
- make

### Build command
```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/HelloWorld
make
```

### Build outputs
- hello_world.elf (debuggable image)
- hello_world.hex (common flashing image)
- hello_world.bin (raw binary)
- hello_world.map (linker map)

## Flash to Wio-E5 from Windows via USB-C

This section is written for newcomers and assumes only a USB-C cable.

### Option A (recommended for USB-C only): STM32 ROM bootloader over UART
Use this when your Wio-E5 board exposes USB-UART through its USB-C connector.

#### 1. Install tools on Windows
- Install STM32CubeProgrammer (GUI + CLI)
- Install USB-UART driver if needed (CP210x or CH340 depending on board)

#### 2. Enter bootloader mode
Exact button names vary by board revision, but the sequence is usually:
1. Hold BOOT button (or set BOOT0 high)
2. Press and release RESET
3. Release BOOT

If your board has dedicated BOOT and RST buttons, this is quick.

#### 3. Find COM port
Open Device Manager and note COMx for your board.

#### 4. Flash with STM32CubeProgrammer GUI
1. Open STM32CubeProgrammer
2. Select UART connection
3. Set Port = COMx
4. Baud = 115200 (or board-supported value)
5. Connect
6. Open file hello_world.hex
7. Download/Program
8. Reset board in normal boot mode

#### 5. Verify result
Open a serial terminal at 9600 8N1 and confirm repeating output.

### Option B: ST-LINK path (if your dev kit includes onboard ST-LINK)
1. Connect USB-C
2. Open STM32CubeProgrammer
3. Select ST-LINK
4. Connect
5. Program hello_world.hex
6. Reset and test UART

## Quick Verification Checklist
- Build completed with no errors
- Firmware flashed successfully
- UART terminal shows periodic hello message
- Message interval is roughly 1 second

## Common Issues and Fixes

### No serial output
- Wrong COM port
- Wrong baud rate
- TX/RX crossed incorrectly
- Board still in bootloader mode

### Cannot connect in CubeProgrammer (UART)
- Bootloader mode not entered correctly
- Wrong COM or baud
- USB-UART driver missing

### Build error about compiler missing
Install arm-none-eabi toolchain and ensure it is on PATH.

## Where to go next
- Read docs/PROGRAMMERS_GUIDE.md for internals
- Read docs/COOKBOOK.md for practical recipes
- Move to Telemetry project for low-power + LoRaWAN use case
