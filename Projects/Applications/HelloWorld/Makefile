# Makefile for STM32WLxx Hello World

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Project name
TARGET = hello_world

# MCU
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Defines
DEFINES = -DSTM32WLE5xx -DUSE_HAL_DRIVER

# Project root
ROOT = /workspaces/LoRaWan-E5-Node

# MCU
MCU = -mcpu=cortex-m4 -mthumb

# Defines
DEFINES = -DSTM32WLE5xx -DUSE_HAL_DRIVER

# Includes
INC_DIRS = \
	-I./Core/Inc \
	-I$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Inc \
	-I$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Inc/Legacy \
	-I$(ROOT)/Drivers/CMSIS/Device/ST/STM32WLxx/Include \
	-I$(ROOT)/Drivers/CMSIS/Include

# Source files
SRC_FILES = \
	./Core/Src/main.c \
	./Core/Src/system_stm32wlxx.c \
	./Core/Src/stm32wlxx_hal_msp.c \
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
	$(ROOT)/Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_subghz.c

# Startup file
STARTUP = ./startup_stm32wle5jcix.s

# Linker script
LDSCRIPT = ./STM32WLE5JCIX_FLASH.ld

# Flags
CFLAGS = $(MCU) $(DEFINES) $(INC_DIRS) -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fstack-usage
LDFLAGS = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(TARGET).map

# Objects
OBJ_FILES = $(SRC_FILES:.c=.o) $(STARTUP:.s=.o)

# Linker script
LDSCRIPT = ./STM32WLE5JCIX_FLASH.ld

# Flags
CFLAGS = $(MCU) $(DEFINES) $(INC_DIRS) -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fstack-usage
LDFLAGS = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(TARGET).map

# Objects
OBJ_FILES = $(SRC_FILES:.c=.o) $(STARTUP:.s=.o)

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

.PHONY: all clean