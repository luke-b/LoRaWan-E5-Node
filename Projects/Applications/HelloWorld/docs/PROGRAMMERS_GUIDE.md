# HelloWorld Programmers Guide

## Architecture overview
This project is intentionally small:
- HAL startup and clock config
- USART1 initialization
- Blocking transmit in main loop
- 1 second delay

Main implementation files:
- Core/Src/main.c
- Core/Src/stm32wlxx_hal_msp.c
- Core/Src/system_stm32wlxx.c
- Core/Inc/main.h

Build control:
- Makefile
- STM32WLE5JCIX_FLASH.ld
- startup_stm32wle5jcix.s

## Boot flow
1. Reset handler from startup_stm32wle5jcix.s
2. HAL_Init
3. SystemClock_Config
4. GPIO/UART peripheral init
5. Infinite loop with HAL_UART_Transmit and HAL_Delay

## Design choices
- Blocking UART is used for clarity and deterministic behavior.
- Minimal source list in Makefile keeps build understandable.
- No RTOS, no interrupts needed for baseline test.

## Key configuration points

### MCU and defines
Makefile should include:
- -DSTM32WLE5xx
- -DUSE_HAL_DRIVER

### Include paths
HAL and CMSIS include paths must point to workspace Drivers tree.

### Linker script
STM32WLE5JCIX_FLASH.ld must match your exact target memory layout.

## Extending the example

### Change message period
Adjust HAL_Delay value in main loop.

### Change baud rate
Update UART init structure in main.c.

### Add receive support
Add HAL_UART_Receive or interrupt/DMA based RX.

## Coding conventions for this app
- Keep startup path simple.
- Avoid unnecessary middleware here.
- Treat this as a board-smoke-test app.

## Test strategy
- Build test: make
- Flash test: verify program starts
- Runtime test: observe UART line at expected cadence

## Common integration pitfalls
- Wrong part macro (must be STM32WLE5xx)
- Missing startup file
- Wrong linker script path
- Wrong UART pin mapping for board variant
