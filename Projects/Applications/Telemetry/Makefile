# Makefile for STM32WLxx Telemetry

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Project name
# Support BUILD_PROFILE=emulation for Renode CI builds
BUILD_PROFILE ?= default

ifeq ($(BUILD_PROFILE),emulation)
  TARGET = telemetry_emulation
  DEFINES = -DSTM32WLE5xx -DUSE_HAL_DRIVER -DDEBUG -DEMULATION_BUILD
else
  TARGET = telemetry
  DEFINES = -DSTM32WLE5xx -DUSE_HAL_DRIVER -DDEBUG
endif

# MCU
MCU = -mcpu=cortex-m4 -mthumb

# Project root — derived from this Makefile's location so it works both on
# the host (/workspaces/LoRaWan-E5-Node) and inside Docker (/workspace).
ROOT = $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../..)

# Includes
INC_DIRS = \
	-I./Core/Inc \
	-I./LoRaWAN/App \
	-I./LoRaWAN/Target \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/Mac \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/Mac/Region \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler/Packages \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/Crypto \
	-I$(ROOT)/Middlewares/Third_Party/LoRaWAN/Utilities \
	-I$(ROOT)/Middlewares/Third_Party/SubGHz_Phy \
	-I$(ROOT)/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver \
	-I$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Inc \
	-I$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Inc/Legacy \
	-I$(ROOT)/Drivers/BSP/STM32WLxx_LoRa_E5_mini \
	-I$(ROOT)/Drivers/CMSIS/Device/ST/STM32WLxx/Include \
	-I$(ROOT)/Drivers/CMSIS/Include \
	-I$(ROOT)/Utilities/misc \
	-I$(ROOT)/Utilities/lpm/tiny_lpm \
	-I$(ROOT)/Utilities/sequencer \
	-I$(ROOT)/Utilities/timer \
	-I$(ROOT)/Utilities/trace \
	-I$(ROOT)/Utilities/trace/adv_trace

# Source files
# ── Source files ─────────────────────────────────────────────────────────────
ifeq ($(BUILD_PROFILE),emulation)
# Emulation build: only the harness + pure logic (no HAL, no LoRaWAN stack)
SRC_FILES = \
	./Core/Src/system_stm32wlxx.c \
	./Core/Src/main_emulation.c \
	./Core/Src/telemetry_logic.c \
	./Core/Src/telemetry_app.c
else
# Default (hardware) build: full source list
SRC_FILES = \
	./Core/Src/main.c \
	./Core/Src/system_stm32wlxx.c \
	./Core/Src/telemetry_logic.c \
	./Core/Src/telemetry_app.c \
	./Core/Src/stm32wlxx_hal_msp.c \
	./Core/Src/sys_app.c \
	./Core/Src/adc_if.c \
	./Core/Src/timer_if.c \
	./Core/Src/sys_debug.c \
	./Core/Src/sys_sensors.c \
	./Core/Src/subghz.c \
	./Core/Src/dma.c \
	./Core/Src/usart.c \
	./Core/Src/usart_if.c \
	./LoRaWAN/App/app_lorawan.c \
	./LoRaWAN/App/lora_app.c \
	./LoRaWAN/App/lora_info.c \
	./LoRaWAN/Target/radio_board_if.c \
	$(wildcard $(ROOT)/Middlewares/Third_Party/LoRaWAN/Mac/*.c) \
	$(wildcard $(ROOT)/Middlewares/Third_Party/LoRaWAN/Mac/Region/*.c) \
	$(wildcard $(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler/*.c) \
	$(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpCompliance.c \
	$(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpClockSync.c \
	$(ROOT)/Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpRemoteMcastSetup.c \
	$(wildcard $(ROOT)/Middlewares/Third_Party/LoRaWAN/Crypto/*.c) \
	$(wildcard $(ROOT)/Middlewares/Third_Party/LoRaWAN/Utilities/*.c) \
	$(wildcard $(ROOT)/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/*.c) \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_gpio.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_cortex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_exti.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_adc.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_adc_ex.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_lptim.c \
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_subghz.c \
	$(ROOT)/Drivers/BSP/STM32WLxx_LoRa_E5_mini/stm32wlxx_LoRa_E5_mini.c \
	$(ROOT)/Drivers/BSP/STM32WLxx_LoRa_E5_mini/stm32wlxx_LoRa_E5_mini_radio.c \
	$(ROOT)/Utilities/lpm/tiny_lpm/stm32_lpm.c \
	$(ROOT)/Utilities/misc/stm32_mem.c \
	$(ROOT)/Utilities/misc/stm32_systime.c \
	$(ROOT)/Utilities/misc/stm32_tiny_vsnprintf.c \
	$(ROOT)/Utilities/sequencer/stm32_seq.c \
	$(ROOT)/Utilities/timer/stm32_timer.c \
	$(ROOT)/Utilities/trace/adv_trace/stm32_adv_trace.c
endif

# Startup file
STARTUP = ./startup_stm32wle5jcix.s

# Linker script
LDSCRIPT = ./STM32WLE5JCIX_FLASH.ld

# Flags
CFLAGS = $(MCU) $(DEFINES) $(INC_DIRS) -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fstack-usage
LDFLAGS = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(TARGET).map

# Objects
ifeq ($(BUILD_PROFILE),emulation)
OBJ_FILES = $(SRC_FILES:.c=.o) $(STARTUP:.s=.o)
else
OBJ_FILES = $(SRC_FILES:.c=.o) $(STARTUP:.s=.o)
endif

# Rules
all: $(TARGET).elf

$(TARGET).elf: $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O ihex $@ $(TARGET).hex
	$(OBJCOPY) -O binary $@ $(TARGET).bin
	$(SIZE) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(MCU) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) $(TARGET).elf $(TARGET).hex $(TARGET).bin $(TARGET).map
	rm -f telemetry_emulation.elf telemetry_emulation.hex telemetry_emulation.bin telemetry_emulation.map

# ─── Unit tests (host-native, no HW required) ──────────────────────────────────
UNIT_TEST_DIR  = Emulation/tests/unit
UNIT_TEST_APP_SRC   = $(UNIT_TEST_DIR)/test_telemetry_app.c \
                      ./Core/Src/telemetry_app.c
UNIT_TEST_APP_BIN   = $(UNIT_TEST_DIR)/test_telemetry_app
UNIT_TEST_LOGIC_SRC = $(UNIT_TEST_DIR)/test_telemetry_logic.c \
                      ./Core/Src/telemetry_logic.c
UNIT_TEST_LOGIC_BIN = $(UNIT_TEST_DIR)/test_telemetry_logic

test-unit:
	@echo "[unit] Compiling telemetry_app unit tests..."
	gcc -std=c11 -Wall -Wextra -g \
	    -I./Core/Inc \
	    -o $(UNIT_TEST_APP_BIN) \
	    $(UNIT_TEST_APP_SRC)
	@echo "[unit] Compiling telemetry_logic unit tests..."
	gcc -std=c11 -Wall -Wextra -g \
	    -I./Core/Inc \
	    -o $(UNIT_TEST_LOGIC_BIN) \
	    $(UNIT_TEST_LOGIC_SRC)
	@echo "[unit] Running telemetry_app unit tests..."
	$(UNIT_TEST_APP_BIN)
	@echo "[unit] Running telemetry_logic unit tests..."
	$(UNIT_TEST_LOGIC_BIN)

clean-unit:
	rm -f $(UNIT_TEST_APP_BIN) $(UNIT_TEST_LOGIC_BIN)