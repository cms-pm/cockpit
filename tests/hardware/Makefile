# Makefile for I2C Peripheral Validation Test
# Phase 4.8.2: Hardware validation focus

# Target configuration
TARGET = i2c_peripheral_validation
MCU = STM32G474xx

# Directories
PROJECT_ROOT = ../..
BUILD_DIR = build
SRC_DIR = .
HAL_DIR = $(PROJECT_ROOT)/lib/hal/stm32g4xx
DIAG_DIR = $(PROJECT_ROOT)/lib/vm_bootloader

# Compiler settings
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -O0 -g -fdata-sections -ffunction-sections -Wall
CFLAGS += -D$(MCU) -DUSE_HAL_DRIVER -DHSE_VALUE=8000000
CFLAGS += -I$(HAL_DIR)/Inc -I$(HAL_DIR)/Inc/Legacy -I$(DIAG_DIR)/include
CFLAGS += -I$(SRC_DIR)

# Linker flags  
LDFLAGS = $(CFLAGS) -specs=nano.specs -T$(HAL_DIR)/Ldscripts/STM32G474CEUx_FLASH.ld
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# Source files
SOURCES = $(SRC_DIR)/$(TARGET).c
SOURCES += $(DIAG_DIR)/src/bootloader_diagnostics.c
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal.c
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal_cortex.c
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal_i2c.c
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal_gpio.c  
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal_rcc.c
SOURCES += $(HAL_DIR)/Src/stm32g4xx_hal_uart.c

# Object files
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

# Create build directory
$(BUILD_DIR):
	mkdir -p $(dir $(OBJECTS))

# Compile source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

# Create binary
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Flash to target (using st-flash)
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $< 0x08000000

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Show help
help:
	@echo "Available targets:"
	@echo "  all     - Build project" 
	@echo "  flash   - Flash to STM32G474"
	@echo "  clean   - Clean build files"
	@echo "  help    - Show this help"

.PHONY: all flash clean help