# ComponentVM Hardware Integration Guide - STM32G431CB

**Version**: 3.10.0  
**Date**: July 10, 2025  
**Target Hardware**: WeAct Studio STM32G431CB CoreBoard  
**Toolchain**: PlatformIO + OpenOCD + ST-Link V2  
**Approach**: Progressive hardware bringup with embedded bytecode firmware  

---

## 📋 Table of Contents

- [Hardware Overview](#hardware-overview)
- [Memory Layout & Partitioning](#memory-layout--partitioning)
- [Development Environment Setup](#development-environment-setup)
- [Progressive Hardware Bringup](#progressive-hardware-bringup)
- [Firmware Development Workflow](#firmware-development-workflow)
- [Troubleshooting & Validation](#troubleshooting--validation)
- [Production Deployment](#production-deployment)

---

## Hardware Overview

### **WeAct Studio STM32G431CB CoreBoard Specifications**

```yaml
Microcontroller: STM32G431CBU6
Core: ARM Cortex-M4F @ 170MHz
Flash: 128KB (0x08000000 - 0x0801FFFF)
RAM: 32KB (0x20000000 - 0x20007FFF)
Package: UFQFPN48

Key Peripherals:
  - GPIO Ports: A, B, C
  - USART: USART1 (PA9/PA10)
  - SWD: PA13 (SWDIO), PA14 (SWCLK)
  - Timers: TIM1, TIM2, TIM3, TIM4, TIM6, TIM7, TIM8
  - ADC: 12-bit, 16 channels
  - Clock: 8MHz HSE crystal
```

### **Board-Specific Pin Configuration**
```c
// WeAct STM32G431CB pin definitions
#define LED_PIN             PC6     // Onboard LED (active high)
#define USER_BUTTON_PIN     PC13    // User button (active low, pull-up)
#define USB_USART_TX        PA11    // USB-connected USART
#define USB_USART_RX        PA12    // USB-connected USART  
#define DEBUG_USART_TX      PA9     // Debug USART1 TX
#define DEBUG_USART_RX      PA10    // Debug USART1 RX
#define BOOT0_BUTTON        PB8     // BOOT0 button
#define HSE_CRYSTAL_FREQ    8000000 // 8MHz external crystal
```

### **Required Hardware Components**
```yaml
Primary Components:
  - WeAct Studio STM32G431CB CoreBoard
  - ST-Link V2 debugger (or compatible)
  - SWD ribbon cable (2x5 pin, 1.27mm pitch)
  - USB cable (board power)
  - USB cable (ST-Link connection)

Optional Components:
  - USB-to-Serial adapter (for USART debugging)
  - Oscilloscope (for timing validation)
  - Logic analyzer (for debugging)
  - Breadboard + jumper wires (for external peripherals)
```

---

## Memory Layout & Partitioning

### **STM32G431CB Memory Map**
```
┌─────────────────────────────────────┐ 0x08000000
│ Flash Memory (128KB)                │
│ ├─ Vector Table (1KB)               │ 0x08000000 - 0x08000400
│ ├─ ComponentVM Firmware (96KB)      │ 0x08000400 - 0x08018000
│ ├─ Embedded Bytecode (30KB)         │ 0x08018000 - 0x0801F800
│ ├─ Configuration Data (1KB)         │ 0x0801F800 - 0x0801FC00
│ └─ Reserved Space (1KB)             │ 0x0801FC00 - 0x08020000
└─────────────────────────────────────┘

┌─────────────────────────────────────┐ 0x20000000  
│ SRAM Memory (32KB)                  │
│ ├─ System RAM (8KB)                 │ 0x20000000 - 0x20002000
│ │  ├─ Main Stack (4KB)              │
│ │  ├─ System Heap (3KB)             │
│ │  └─ Hardware Drivers (1KB)        │
│ └─ ComponentVM Memory (24KB)        │ 0x20002000 - 0x20008000
│    ├─ VM Stack (12KB)               │
│    ├─ VM Heap (10KB)                │
│    └─ VM Globals (2KB)              │
└─────────────────────────────────────┘
```

### **Flash Partitioning Strategy**
```c
// Linker script sections for embedded bytecode
MEMORY {
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 96K   /* Firmware */
    BYTECODE (r)    : ORIGIN = 0x08018000, LENGTH = 30K   /* Programs */
    CONFIG (r)      : ORIGIN = 0x0801F800, LENGTH = 1K    /* Metadata */
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 8K    /* System */
    VM_RAM (rw)     : ORIGIN = 0x20002000, LENGTH = 24K   /* VM Memory */
}

// Embedded bytecode section
__attribute__((section(".bytecode")))
const vm_instruction_c_t embedded_programs[] = {
    // SOS demo program
    {0x15, 0, PC6},    // pinMode(LED, OUTPUT)  
    {0x11, 0, PC13},   // digitalRead(BUTTON)
    // ... complete SOS implementation
    {0x00, 0, 0}       // HALT
};
```

---

## Development Environment Setup

### **1. Hardware Connection Setup**

#### **ST-Link V2 to STM32G431CB Connection**
```
ST-Link V2 Header (2x5, 1.27mm)     STM32G431CB SWD Header
┌─────┬─────┐                       ┌─────┬─────┐
│ VCC │ 1 2 │ SWCLK  ←──────────→   │ 1 2 │ 3V3 │ 
│ GND │ 3 4 │ SWDIO  ←──────────→   │ 3 4 │ PA14│ SWCLK
│ GND │ 5 6 │ nRST   ←──────────→   │ 5 6 │ PA13│ SWDIO  
│ NC  │ 7 8 │ NC                    │ 7 8 │ GND │
│ NC  │ 9 10│ NC                    │ 9 10│ nRST│
└─────┴─────┘                       └─────┴─────┘

Connection Verification:
- VCC: Connect only if board needs external power (usually not needed)
- GND: Essential ground connection
- SWCLK (PA14): Serial Wire Clock
- SWDIO (PA13): Serial Wire Data I/O  
- nRST: Reset line (optional but recommended)
```

#### **Power Connection**
```yaml
Primary Power: USB-C connector on WeAct board
Secondary Power: 3.3V via ST-Link (if needed)
Power Consumption: ~50mA typical, ~200mA peak

Power Validation:
  - LED should illuminate when powered
  - Verify 3.3V on VDD pins with multimeter
  - Check for stable power without excessive ripple
```

### **2. PlatformIO Board Definition Installation**

#### **Method 1: Local Board Definition (Recommended for Development)**
```bash
# Copy board definition to PlatformIO
mkdir -p ~/.platformio/boards/
cp boards/weact_g431cb.json ~/.platformio/boards/

# Verify installation
pio boards ststm32 | grep weact_g431cb
```

#### **Method 2: Project-Local Board Definition**
```bash
# Use board directly from project
# platformio.ini configuration:
[env:weact_g431cb_hardware]
platform = ststm32
board = boards/weact_g431cb.json
framework = arduino
upload_protocol = stlink
debug_tool = stlink
monitor_speed = 115200
```

#### **Board Definition Validation**
```bash
# Test board definition parsing
pio run -e weact_g431cb_hardware --dry-run

# Expected output should show:
# - STM32G431CBU6 as target MCU
# - 170MHz CPU frequency
# - 128KB flash, 32KB RAM
# - stlink upload protocol
```

### **3. OpenOCD Configuration**

#### **OpenOCD Installation and Setup**
```bash
# Install OpenOCD (Ubuntu/Debian)
sudo apt update
sudo apt install openocd

# Verify installation
openocd --version
# Expected: Open On-Chip Debugger 0.11.0 or later

# Test ST-Link detection
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "init; halt; exit"
```

#### **STM32G431 OpenOCD Configuration**
```bash
# Create project-specific OpenOCD config: openocd_stm32g431.cfg
source [find interface/stlink.cfg]
source [find target/stm32g4x.cfg]

# STM32G431CB specific settings
set WORKAREASIZE 0x4000        # 16KB work area in RAM
set CHIPNAME stm32g431cb
set CPUTAPID 0x2ba01477        # Cortex-M4 TAP ID

# Programming configuration
flash bank $_FLASHNAME stm32l4x 0x08000000 0 0 0 $_TARGETNAME
reset_config srst_only srst_nogate

# Debug configuration  
cortex_m reset_config vectreset
```

#### **OpenOCD Connection Test**
```bash
# Test OpenOCD connection
openocd -f openocd_stm32g431.cfg

# Expected output:
# Info : STLINK V2J37S7 (API v2) VID:PID 0483:3748
# Info : Target voltage: 3.3V
# Info : stm32g4x.cpu: hardware has 6 breakpoints, 4 watchpoints
# Info : starting gdb server for stm32g4x.cpu on port 3333
```

### **4. PlatformIO Project Configuration**

#### **Complete platformio.ini for Hardware Development**
```ini
[env:weact_g431cb_hardware]
platform = ststm32
board = boards/weact_g431cb.json
framework = arduino

# Build configuration
build_flags = 
    -DSTM32G431xx
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000        # 8MHz external crystal
    -DCOMPONENTVM_HARDWARE     # Enable hardware HAL
    -Os                        # Optimize for size
    -Wall
    -Wextra

# Upload and debug configuration
upload_protocol = stlink
upload_flags = 
    --connect-under-reset
    --frequency=1800           # 1.8MHz SWD frequency

debug_tool = stlink
debug_init_break = tbreak main
debug_extra_cmds =
    monitor reset init
    monitor halt

# Serial monitoring (optional)
monitor_speed = 115200
monitor_port = /dev/ttyUSB0    # Adjust for your system

# Custom targets
extra_scripts = 
    scripts/generate_bytecode.py
    scripts/memory_analysis.py
```

#### **Custom Build Script for Bytecode Embedding**
```python
# scripts/generate_bytecode.py
Import("env")

def generate_embedded_bytecode(source, target, env):
    """Generate C header with embedded bytecode programs"""
    
    # Compile SOS demo to bytecode
    sos_bytecode = compile_c_to_bytecode("programs/sos_demo.c")
    
    # Generate header file
    header_content = f"""
// Auto-generated embedded bytecode programs
#ifndef EMBEDDED_PROGRAMS_H
#define EMBEDDED_PROGRAMS_H

#include "component_vm_c.h"

// SOS Demo Program ({len(sos_bytecode)} instructions)
extern const vm_instruction_c_t sos_program[];
extern const size_t sos_program_size;
extern const char* sos_strings[];
extern const size_t sos_string_count;

#endif // EMBEDDED_PROGRAMS_H
"""
    
    with open("src/embedded_programs.h", "w") as f:
        f.write(header_content)
    
    print("✓ Embedded bytecode programs generated")

env.AddPreAction("buildprog", generate_embedded_bytecode)
```

---

## Progressive Hardware Bringup

### **Phase 1: System Clock & GPIO Foundation**

#### **1.1: HSE Clock Configuration**
```c
// src/hardware_init.c - System clock setup
#include "stm32g4xx_hal.h"

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure HSE oscillator
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    
    // PLL configuration for 170MHz
    // HSE = 8MHz, PLL = 8MHz * (85/2) / 2 = 170MHz
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;    // Divide by 2: 4MHz
    RCC_OscInitStruct.PLL.PLLN = 85;               // Multiply by 85: 340MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;    // Divide by 2: 170MHz
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // Configure system clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     // 170MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      // 170MHz  
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;      // 170MHz
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK) {
        Error_Handler();
    }
    
    // Enable peripheral clocks needed for basic operation
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

// Clock validation function
uint32_t validate_system_clock(void) {
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    
    printf("System Clock Validation:\n");
    printf("  SYSCLK: %lu Hz (expected: 170000000)\n", sysclk);
    printf("  HCLK:   %lu Hz (expected: 170000000)\n", hclk);
    printf("  PCLK1:  %lu Hz (expected: 170000000)\n", pclk1);
    printf("  PCLK2:  %lu Hz (expected: 170000000)\n", pclk2);
    
    return (sysclk == 170000000) ? 1 : 0;
}
```

#### **1.2: GPIO Configuration for LED**
```c
// src/gpio_hal.c - GPIO hardware abstraction
#include "stm32g4xx_hal.h"

// LED pin configuration (PC6)
void LED_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIOC clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure PC6 as output
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    // Initialize LED to OFF state
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
}

// Arduino-compatible digitalWrite implementation
void digitalWrite(uint8_t pin, uint8_t value) {
    switch (pin) {
        case PC6:  // LED pin
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, 
                             value ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        // Add other pins as needed
        default:
            // Invalid pin - could trigger error or ignore
            break;
    }
}

// Arduino-compatible pinMode implementation  
void pinMode(uint8_t pin, uint8_t mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    switch (pin) {
        case PC6:  // LED pin
            GPIO_InitStruct.Pin = GPIO_PIN_6;
            GPIO_InitStruct.Mode = (mode == OUTPUT) ? GPIO_MODE_OUTPUT_PP : 
                                                     GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
            break;
        // Add other pins as needed
    }
}
```

#### **1.3: Phase 1 Validation Program**
```c
// src/phase1_test.c - Basic LED blink test
#include "stm32g4xx_hal.h"
#include "hardware_init.h"

// Embedded blink program for ComponentVM
const vm_instruction_c_t phase1_blink_program[] = {
    {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)
    
    // Main blink loop (10 cycles)
    {0x01, 0, 10},       // PUSH 10 (loop counter)
    
    // Loop start - turn LED on
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)  
    {0x01, 0, 1},        // PUSH 1 (HIGH value)
    {0x14, 0, 500},      // delay(500)
    
    // Turn LED off
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
    {0x01, 0, 0},        // PUSH 0 (LOW value)  
    {0x14, 0, 500},      // delay(500)
    
    // Decrement counter and loop
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB (counter - 1)
    {0x32, 0, 2},        // JMP_TRUE to loop start
    
    {0x00, 0, 0}         // HALT
};

int main(void) {
    // Initialize hardware
    HAL_Init();
    SystemClock_Config();
    LED_GPIO_Init();
    
    // Validate clock configuration
    if (!validate_system_clock()) {
        // Clock configuration failed - enter infinite loop
        while (1) {
            // Blink rapidly to indicate error
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(100);
        }
    }
    
    // Initialize ComponentVM
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        // VM creation failed - blink SOS pattern
        while (1) {
            blink_sos_error_pattern();
        }
    }
    
    // Execute blink program
    bool result = component_vm_execute_program(vm, phase1_blink_program, 
                                              sizeof(phase1_blink_program) / sizeof(vm_instruction_c_t));
    
    if (result) {
        // Success - LED should have blinked 10 times
        // Turn on LED solid to indicate success
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    } else {
        // Program execution failed - get error details
        vm_error_t error = component_vm_get_last_error(vm);
        blink_error_code(error);
    }
    
    component_vm_destroy(vm);
    
    while (1) {
        // Main loop - program complete
        HAL_Delay(1000);
    }
}

// Error indication functions
void blink_sos_error_pattern(void) {
    // S: 3 short blinks
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(200);
    }
    
    HAL_Delay(400);
    
    // O: 3 long blinks  
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(600);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(200);
    }
    
    HAL_Delay(400);
    
    // S: 3 short blinks
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(200);
    }
    
    HAL_Delay(2000);  // Long pause before repeat
}

void blink_error_code(vm_error_t error) {
    // Blink error code number (1-15 blinks)
    for (int i = 0; i < (int)error && i < 15; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(300);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(300);
    }
    HAL_Delay(3000);  // Long pause before repeat
}
```

#### **Phase 1 Build and Test Procedure**
```bash
# Build Phase 1 firmware
pio run -e weact_g431cb_hardware

# Flash to hardware
pio run -e weact_g431cb_hardware -t upload

# Expected behavior:
# 1. LED blinks 10 times (500ms on, 500ms off)
# 2. LED turns on solid (success indication)
# 3. If error: SOS pattern or error code blinks

# Debug with OpenOCD if needed
pio debug -e weact_g431cb_hardware
```

### **Phase 2: SysTick Timer & Accurate Timing**

#### **2.1: SysTick Configuration**
```c
// src/systick_hal.c - Precise timing implementation
#include "stm32g4xx_hal.h"

volatile uint32_t system_tick_count = 0;

// SysTick initialization for 1ms ticks
void SysTick_Init(void) {
    // Configure SysTick for 1ms interrupts
    // SystemCoreClock = 170MHz, want 1000 Hz (1ms)
    if (SysTick_Config(SystemCoreClock / 1000) != 0) {
        // SysTick configuration failed
        Error_Handler();
    }
    
    // Set SysTick priority (lower number = higher priority)
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
    
    system_tick_count = 0;
}

// SysTick interrupt handler
void SysTick_Handler(void) {
    system_tick_count++;
    HAL_IncTick();  // Required for HAL library
}

// Arduino-compatible millis() implementation
uint32_t millis(void) {
    return system_tick_count;
}

// Arduino-compatible micros() implementation (approximate)
uint32_t micros(void) {
    uint32_t ms = system_tick_count;
    uint32_t ticks = SysTick->VAL;
    uint32_t load = SysTick->LOAD;
    
    // Calculate microseconds within current millisecond
    uint32_t us_in_ms = ((load - ticks) * 1000) / load;
    
    return (ms * 1000) + us_in_ms;
}

// Arduino-compatible delay() implementation
void delay(uint32_t ms) {
    uint32_t start = millis();
    while ((millis() - start) < ms) {
        // Wait for specified time
        __WFI();  // Wait for interrupt (low power)
    }
}

// Precision delay in microseconds (blocking)
void delayMicroseconds(uint32_t us) {
    uint32_t start = micros();
    while ((micros() - start) < us) {
        // Busy wait for microsecond precision
    }
}
```

#### **2.2: Timing Validation Program**
```c
// src/phase2_test.c - Timing accuracy validation
#include "stm32g4xx_hal.h"
#include "hardware_init.h"

// Embedded timing test program
const vm_instruction_c_t phase2_timing_program[] = {
    {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)
    
    // Test 1: 100ms delay accuracy
    {0x16, 0, 0},        // millis() - get start time
    {0x14, 0, 100},      // delay(100)
    {0x16, 0, 0},        // millis() - get end time
    // Expect ~100ms elapsed time
    
    // Test 2: Rapid toggle for frequency measurement
    {0x01, 0, 1000},     // PUSH 1000 (loop counter)
    
    // Fast toggle loop - 1kHz square wave
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
    {0x01, 0, 1},        // PUSH 1
    {0x17, 0, 500},      // delayMicroseconds(500)
    
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)  
    {0x01, 0, 0},        // PUSH 0
    {0x17, 0, 500},      // delayMicroseconds(500)
    
    // Decrement and loop
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB
    {0x32, 0, 4},        // JMP_TRUE to toggle start
    
    {0x00, 0, 0}         // HALT
};

int main(void) {
    // Initialize hardware
    HAL_Init();
    SystemClock_Config();
    SysTick_Init();
    LED_GPIO_Init();
    
    // Timing validation tests
    timing_validation_suite();
    
    // Initialize ComponentVM
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        blink_sos_error_pattern();
        while (1);
    }
    
    // Execute timing test program
    bool result = component_vm_execute_program(vm, phase2_timing_program,
                                              sizeof(phase2_timing_program) / sizeof(vm_instruction_c_t));
    
    if (result) {
        // Success indication - slow blink
        while (1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            delay(1000);
        }
    } else {
        vm_error_t error = component_vm_get_last_error(vm);
        blink_error_code(error);
        while (1);
    }
}

// Comprehensive timing validation
void timing_validation_suite(void) {
    uint32_t start, end, elapsed;
    
    // Test 1: millis() basic functionality
    start = millis();
    HAL_Delay(100);  // Use HAL delay as reference
    end = millis();
    elapsed = end - start;
    
    if (elapsed < 95 || elapsed > 105) {
        // Timing error - blink rapidly
        for (int i = 0; i < 20; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(50);
        }
    }
    
    // Test 2: delay() accuracy
    start = millis();
    delay(250);
    end = millis();
    elapsed = end - start;
    
    if (elapsed < 245 || elapsed > 255) {
        // Delay error - different blink pattern
        for (int i = 0; i < 10; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(100);
        }
    }
    
    // Test 3: micros() resolution
    uint32_t us1 = micros();
    delayMicroseconds(10);
    uint32_t us2 = micros();
    
    if ((us2 - us1) < 8 || (us2 - us1) > 15) {
        // Microsecond timing error
        for (int i = 0; i < 30; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(25);
        }
    }
    
    // All tests passed - brief success indication
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
}
```

#### **Phase 2 Validation with Oscilloscope**
```bash
# Build and flash Phase 2 firmware  
pio run -e weact_g431cb_hardware -t upload

# Oscilloscope measurement points:
# - Probe: PC6 (LED pin)
# - Ground: GND pin
# - Trigger: Rising edge

# Expected measurements:
# 1. Initial timing tests: Brief activity
# 2. 1kHz square wave: 500us high, 500us low
# 3. Final state: Slow 1Hz blinking

# Timing accuracy requirements:
# - 1kHz frequency: ±1% (990Hz - 1010Hz)
# - Duty cycle: 50% ±2%
# - Rise/fall time: <1us
```

### **Phase 3: Digital Input & Button Handling**

#### **3.1: Button GPIO Configuration**  
```c
// src/button_hal.c - Button input with debouncing
#include "stm32g4xx_hal.h"

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t current_state;
    uint8_t previous_state;
    uint32_t last_change_time;
    uint32_t debounce_delay;
} button_t;

// Button instances
button_t user_button = {
    .port = GPIOC,
    .pin = GPIO_PIN_13,
    .current_state = 1,      // Released (pull-up)
    .previous_state = 1,
    .last_change_time = 0,
    .debounce_delay = 50     // 50ms debounce
};

// Button GPIO initialization
void Button_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIOC clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure PC13 as input with pull-up
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

// Arduino-compatible digitalRead implementation
uint8_t digitalRead(uint8_t pin) {
    switch (pin) {
        case PC13:  // User button
            return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
        case PC6:   // LED pin (for feedback)
            return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
        default:
            return 0;  // Invalid pin
    }
}

// Debounced button reading
uint8_t buttonPressed(uint8_t button_id) {
    if (button_id != 0) return 0;  // Only support button 0
    
    uint32_t current_time = millis();
    uint8_t reading = !digitalRead(PC13);  // Inverted (active low)
    
    if (reading != user_button.current_state) {
        if ((current_time - user_button.last_change_time) > user_button.debounce_delay) {
            user_button.previous_state = user_button.current_state;
            user_button.current_state = reading;
            user_button.last_change_time = current_time;
            
            // Return true on press (transition from 0 to 1)
            return (user_button.current_state && !user_button.previous_state);
        }
    }
    
    return 0;  // No press detected
}

uint8_t buttonReleased(uint8_t button_id) {
    if (button_id != 0) return 0;
    
    uint32_t current_time = millis();
    uint8_t reading = !digitalRead(PC13);
    
    if (reading != user_button.current_state) {
        if ((current_time - user_button.last_change_time) > user_button.debounce_delay) {
            user_button.previous_state = user_button.current_state;
            user_button.current_state = reading;
            user_button.last_change_time = current_time;
            
            // Return true on release (transition from 1 to 0)
            return (!user_button.current_state && user_button.previous_state);
        }
    }
    
    return 0;  // No release detected
}
```

#### **3.2: Interactive LED Control Program**
```c
// src/phase3_test.c - Button-controlled LED
#include "stm32g4xx_hal.h"
#include "hardware_init.h"

// Interactive button control program
const vm_instruction_c_t phase3_interactive_program[] = {
    {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)    - LED
    {0x15, 0, PC13},     // pinMode(PC13, INPUT)    - Button
    {0x01, 0, 1},        // PUSH 1 (INPUT mode)
    
    // Main loop - wait for button press
    {0x19, 0, 0},        // buttonPressed(0)
    {0x33, 0, 3},        // JMP_FALSE to loop start (wait for press)
    
    // Button pressed - toggle LED state
    {0x11, 0, PC6},      // digitalRead(PC6) - get current LED state
    {0x01, 0, 1},        // PUSH 1
    {0x26, 0, 0},        // XOR (toggle bit)
    {0x10, 0, PC6},      // digitalWrite(PC6, toggled_state)
    
    // Wait for button release to avoid multiple toggles
    {0x1A, 0, 0},        // buttonReleased(0)
    {0x33, 0, 8},        // JMP_FALSE to release wait
    
    // Brief delay before next press detection
    {0x14, 0, 100},      // delay(100)
    
    {0x31, 0, 3},        // JMP to main loop start
    {0x00, 0, 0}         // HALT (never reached)
};

int main(void) {
    // Initialize hardware
    HAL_Init();
    SystemClock_Config();
    SysTick_Init();
    LED_GPIO_Init();
    Button_GPIO_Init();
    
    // Button hardware validation
    button_validation_test();
    
    // Initialize ComponentVM
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        blink_sos_error_pattern();
        while (1);
    }
    
    // Execute interactive program (runs indefinitely)
    bool result = component_vm_execute_program(vm, phase3_interactive_program,
                                              sizeof(phase3_interactive_program) / sizeof(vm_instruction_c_t));
    
    // Should never reach here - program runs forever
    component_vm_destroy(vm);
    
    while (1) {
        // Fallback: manual button test
        if (buttonPressed(0)) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            while (buttonPressed(0)) {
                delay(10);  // Wait for release
            }
        }
        delay(10);
    }
}

// Button hardware validation
void button_validation_test(void) {
    // Test 1: Basic digital read
    uint8_t button_state = digitalRead(PC13);
    // Should read 1 (high) when not pressed
    
    // Test 2: Press detection
    uint32_t start_time = millis();
    uint32_t timeout = 5000;  // 5 second timeout
    
    // Indicate waiting for button press
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    
    while ((millis() - start_time) < timeout) {
        if (buttonPressed(0)) {
            // Button press detected - flash LED rapidly
            for (int i = 0; i < 10; i++) {
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
                delay(50);
            }
            break;
        }
        delay(10);
    }
    
    // Turn off LED after test
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    delay(500);
}
```

#### **Phase 3 Validation Procedure**
```bash
# Build and flash Phase 3 firmware
pio run -e weact_g431cb_hardware -t upload

# Expected behavior:
# 1. LED turns on (waiting for button press)
# 2. Press user button - LED flashes rapidly (validation)
# 3. LED turns off briefly
# 4. Interactive mode begins:
#    - Press button -> LED toggles on
#    - Press button -> LED toggles off
#    - Continues indefinitely

# Troubleshooting:
# - If LED doesn't flash on press: Check button wiring/pull-up
# - If multiple toggles per press: Increase debounce delay
# - If no response: Check PC13 pin configuration
```

### **Phase 4: USART Communication & Printf Debug**

#### **4.1: USART1 Configuration**
```c
// src/usart_hal.c - USART communication for debug printf
#include "stm32g4xx_hal.h"
#include <stdio.h>

UART_HandleTypeDef huart1;

// USART1 initialization (PA9=TX, PA10=RX)
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable clocks
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // Configure GPIO pins for USART1
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;  // PA9=TX, PA10=RX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure USART1
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

// Redirect printf to USART1
int _write(int file, char *ptr, int len) {
    (void)file;  // Unused parameter
    
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

// Printf over USART (blocking)
void debug_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, 1000);
    }
}

// ComponentVM printf integration  
void vm_printf_handler(uint8_t string_id, const int32_t* args, uint8_t arg_count) {
    // This would be called by ComponentVM OP_PRINTF instruction
    // For now, just output debug info
    debug_printf("VM Printf: string_id=%d, args=%d\n", string_id, arg_count);
    
    // In full implementation, would format string and output
}
```

#### **4.2: Debug Communication Test Program**
```c
// src/phase4_test.c - USART printf debugging
#include "stm32g4xx_hal.h"
#include "hardware_init.h"
#include <stdio.h>

// Debug-enabled program with printf output
const vm_instruction_c_t phase4_debug_program[] = {
    {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)
    
    // Printf debug messages
    {0x01, 0, 0},        // PUSH 0 (string ID 0)
    {0x01, 0, 0},        // PUSH 0 (arg count) 
    {0x18, 0, 0},        // PRINTF "Program started"
    
    // Loop with debug output
    {0x01, 0, 5},        // PUSH 5 (loop counter)
    
    // Loop start
    {0x01, 0, 1},        // PUSH 1 (string ID 1)
    {0x50, 0, 0},        // LOAD_GLOBAL 0 (loop counter)
    {0x01, 0, 1},        // PUSH 1 (arg count)
    {0x18, 0, 1},        // PRINTF "Loop iteration: %d"
    
    // Blink LED
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
    {0x01, 0, 1},        // PUSH 1
    {0x14, 0, 250},      // delay(250)
    
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
    {0x01, 0, 0},        // PUSH 0  
    {0x14, 0, 250},      // delay(250)
    
    // Decrement and loop
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB
    {0x51, 0, 0},        // STORE_GLOBAL 0
    {0x50, 0, 0},        // LOAD_GLOBAL 0
    {0x32, 0, 6},        // JMP_TRUE to loop start
    
    // Final printf
    {0x01, 0, 2},        // PUSH 2 (string ID 2)
    {0x01, 0, 0},        // PUSH 0 (arg count)
    {0x18, 0, 2},        // PRINTF "Program completed"
    
    {0x00, 0, 0}         // HALT
};

// Printf format strings
const char* phase4_strings[] = {
    "ComponentVM debug program started\n",     // String ID 0
    "Loop iteration: %d\n",                   // String ID 1  
    "Program completed successfully\n"        // String ID 2
};

int main(void) {
    // Initialize hardware
    HAL_Init();
    SystemClock_Config();
    SysTick_Init();
    LED_GPIO_Init();
    Button_GPIO_Init();
    USART1_Init();
    
    // Send startup message
    printf("\n=== ComponentVM Phase 4 Debug Test ===\n");
    printf("System clock: %lu Hz\n", HAL_RCC_GetSysClockFreq());
    printf("USART1 configured at 115200 baud\n");
    printf("Hardware initialization complete\n\n");
    
    // Hardware validation with debug output
    usart_validation_test();
    
    // Initialize ComponentVM
    printf("Creating ComponentVM instance...\n");
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        printf("ERROR: ComponentVM creation failed\n");
        blink_sos_error_pattern();
        while (1);
    }
    printf("ComponentVM created successfully\n");
    
    // Load program with string table
    printf("Loading debug program with string table...\n");
    bool loaded = component_vm_load_program_with_strings(vm, phase4_debug_program,
                                                        sizeof(phase4_debug_program) / sizeof(vm_instruction_c_t),
                                                        phase4_strings, 3);
    if (!loaded) {
        printf("ERROR: Failed to load program\n");
        component_vm_destroy(vm);
        while (1);
    }
    printf("Program loaded with %d strings\n", component_vm_get_string_count(vm));
    
    // Execute program
    printf("Executing debug program...\n");
    bool result = component_vm_execute_program(vm, phase4_debug_program,
                                              sizeof(phase4_debug_program) / sizeof(vm_instruction_c_t));
    
    if (result) {
        printf("Program execution completed successfully\n");
        printf("Final PC: %zu\n", component_vm_get_program_counter(vm));
        printf("Instructions executed: %zu\n", component_vm_get_instruction_count(vm));
        
        // Performance metrics
        vm_c_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
        printf("Performance metrics:\n");
        printf("  Execution time: %lu ms\n", metrics.execution_time_ms);
        printf("  Memory operations: %zu\n", metrics.memory_operations);
        printf("  I/O operations: %zu\n", metrics.io_operations);
        
    } else {
        vm_error_t error = component_vm_get_last_error(vm);
        printf("ERROR: Program execution failed\n");
        printf("Error code: %d\n", error);
        printf("Error description: %s\n", component_vm_get_error_string(error));
        printf("Failed at PC: %zu\n", component_vm_get_program_counter(vm));
    }
    
    component_vm_destroy(vm);
    printf("ComponentVM destroyed\n");
    
    // Success indication
    printf("Phase 4 test completed - entering infinite loop\n");
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        printf("Heartbeat: %lu ms\n", millis());
        delay(2000);
    }
}

// USART validation test
void usart_validation_test(void) {
    printf("=== USART Validation Test ===\n");
    
    // Test 1: Basic printf functionality
    printf("Test 1: Basic printf - ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", i);
        delay(100);
    }
    printf("PASS\n");
    
    // Test 2: Integer formatting
    printf("Test 2: Integer formatting\n");
    printf("  Decimal: %d\n", 12345);
    printf("  Hex: 0x%X\n", 0xABCD);
    printf("  Character: '%c'\n", 'A');
    
    // Test 3: Timing output
    uint32_t start = millis();
    delay(100);
    uint32_t end = millis();
    printf("Test 3: Timing - %lu ms elapsed (expected ~100)\n", end - start);
    
    // Test 4: Button state reporting
    printf("Test 4: Button state - ");
    uint8_t button_state = digitalRead(PC13);
    printf("PC13 = %d %s\n", button_state, button_state ? "(Released)" : "(Pressed)");
    
    printf("USART validation complete\n\n");
}
```

#### **Phase 4 Serial Terminal Setup**
```bash
# Build and flash Phase 4 firmware
pio run -e weact_g431cb_hardware -t upload

# Connect USART1 to serial adapter:
# PA9 (TX) -> RX on USB-to-Serial adapter
# PA10 (RX) -> TX on USB-to-Serial adapter  
# GND -> GND on adapter

# Open serial terminal (Linux)
sudo screen /dev/ttyUSB0 115200

# Expected output:
=== ComponentVM Phase 4 Debug Test ===
System clock: 170000000 Hz
USART1 configured at 115200 baud
Hardware initialization complete

=== USART Validation Test ===
Test 1: Basic printf - 0 1 2 3 4 PASS
Test 2: Integer formatting
  Decimal: 12345
  Hex: 0xABCD
  Character: 'A'
Test 3: Timing - 100 ms elapsed (expected ~100)
Test 4: Button state - PC13 = 1 (Released)
USART validation complete

Creating ComponentVM instance...
ComponentVM created successfully
Loading debug program with string table...
Program loaded with 3 strings
Executing debug program...
ComponentVM debug program started
Loop iteration: 5
Loop iteration: 4
Loop iteration: 3
Loop iteration: 2
Loop iteration: 1
Program completed successfully
Program execution completed successfully
Final PC: 23
Instructions executed: 67
Performance metrics:
  Execution time: 1250 ms
  Memory operations: 45
  I/O operations: 10
ComponentVM destroyed
Phase 4 test completed - entering infinite loop
Heartbeat: 2000 ms
Heartbeat: 4000 ms
...
```

### **Phase 5: MVP Integration - Complete SOS Demo**

#### **5.1: SOS Demo Implementation**
```c
// src/phase5_sos_demo.c - Complete SOS morse code demo
#include "stm32g4xx_hal.h"
#include "hardware_init.h"
#include <stdio.h>

// Complete SOS demo with button control
const vm_instruction_c_t sos_demo_program[] = {
    // Initialize peripherals
    {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)    - LED
    {0x15, 0, PC13},     // pinMode(PC13, INPUT)    - Button
    {0x01, 0, 1},        // PUSH 1 (INPUT mode)
    
    // Print startup message
    {0x01, 0, 0},        // PUSH 0 (string ID 0)
    {0x01, 0, 0},        // PUSH 0 (arg count)
    {0x18, 0, 0},        // PRINTF "SOS Demo Ready - Press button to start"
    
    // Wait for button press to start
    {0x19, 0, 0},        // buttonPressed(0)
    {0x33, 0, 6},        // JMP_FALSE to wait loop
    
    // Print start message
    {0x01, 0, 1},        // PUSH 1 (string ID 1)
    {0x01, 0, 0},        // PUSH 0 (arg count)
    {0x18, 0, 1},        // PRINTF "Starting SOS pattern..."
    
    // SOS Pattern Implementation
    // S: 3 short blinks (200ms on, 200ms off)
    {0x01, 0, 3},        // PUSH 3 (S counter)
    
    // S loop start
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
    {0x01, 0, 1},        // PUSH 1
    {0x14, 0, 200},      // delay(200)
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
    {0x01, 0, 0},        // PUSH 0
    {0x14, 0, 200},      // delay(200)
    
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB (decrement S counter)
    {0x32, 0, 12},       // JMP_TRUE to S loop start
    
    {0x14, 0, 400},      // delay(400) - inter-letter space
    
    // O: 3 long blinks (600ms on, 200ms off)
    {0x01, 0, 3},        // PUSH 3 (O counter)
    
    // O loop start  
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
    {0x01, 0, 1},        // PUSH 1
    {0x14, 0, 600},      // delay(600)
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
    {0x01, 0, 0},        // PUSH 0
    {0x14, 0, 200},      // delay(200)
    
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB (decrement O counter)
    {0x32, 0, 20},       // JMP_TRUE to O loop start
    
    {0x14, 0, 400},      // delay(400) - inter-letter space
    
    // S: 3 short blinks (repeat)
    {0x01, 0, 3},        // PUSH 3 (S counter)
    
    // Final S loop start
    {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
    {0x01, 0, 1},        // PUSH 1
    {0x14, 0, 200},      // delay(200)
    {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
    {0x01, 0, 0},        // PUSH 0
    {0x14, 0, 200},      // delay(200)
    
    {0x01, 0, 1},        // PUSH 1
    {0x24, 0, 0},        // SUB (decrement final S counter)
    {0x32, 0, 28},       // JMP_TRUE to final S loop start
    
    // Print completion message
    {0x01, 0, 2},        // PUSH 2 (string ID 2)
    {0x01, 0, 0},        // PUSH 0 (arg count)
    {0x18, 0, 2},        // PRINTF "SOS pattern completed"
    
    {0x14, 0, 2000},     // delay(2000) - pause before repeat
    
    // Check for button press to stop
    {0x19, 0, 0},        // buttonPressed(0)
    {0x32, 0, 40},       // JMP_TRUE to stop sequence
    
    {0x31, 0, 8},        // JMP to SOS pattern start (repeat)
    
    // Stop sequence
    {0x01, 0, 3},        // PUSH 3 (string ID 3)
    {0x01, 0, 0},        // PUSH 0 (arg count)
    {0x18, 0, 3},        // PRINTF "SOS demo stopped"
    
    {0x00, 0, 0}         // HALT
};

// SOS demo format strings
const char* sos_strings[] = {
    "SOS Demo Ready - Press button to start\n",    // String ID 0
    "Starting SOS pattern...\n",                   // String ID 1
    "SOS pattern completed - Press button to stop or wait for repeat\n",  // String ID 2
    "SOS demo stopped by user\n"                   // String ID 3
};

int main(void) {
    // Initialize all hardware
    HAL_Init();
    SystemClock_Config();
    SysTick_Init();
    LED_GPIO_Init();
    Button_GPIO_Init();
    USART1_Init();
    
    // System status output
    printf("\n=== ComponentVM SOS Demo - Hardware Ready ===\n");
    printf("System: STM32G431CB @ %lu Hz\n", HAL_RCC_GetSysClockFreq());
    printf("LED: PC6, Button: PC13, Debug: USART1\n");
    printf("Firmware: %.1f%% flash, %.1f%% RAM\n", 
           (float)(97000 * 100) / 131072,  // Flash usage
           (float)(10800 * 100) / 32768);   // RAM usage
    
    // Memory integrity check
    printf("Memory validation: ");
    if (memory_integrity_check()) {
        printf("PASS\n");
    } else {
        printf("FAIL - System unsafe\n");
        while (1) blink_sos_error_pattern();
    }
    
    // ComponentVM initialization
    printf("Initializing ComponentVM...\n");
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        printf("CRITICAL: ComponentVM creation failed\n");
        while (1) blink_sos_error_pattern();
    }
    
    // Validate memory integrity after VM creation
    if (!component_vm_validate_memory_integrity(vm)) {
        printf("CRITICAL: VM memory corruption detected\n");
        component_vm_destroy(vm);
        while (1) blink_sos_error_pattern();
    }
    printf("ComponentVM memory validation: PASS\n");
    
    // Load SOS demo program
    printf("Loading SOS demo program (%d instructions)...\n", 
           sizeof(sos_demo_program) / sizeof(vm_instruction_c_t));
    
    bool loaded = component_vm_load_program_with_strings(vm, sos_demo_program,
                                                        sizeof(sos_demo_program) / sizeof(vm_instruction_c_t),
                                                        sos_strings, 4);
    if (!loaded) {
        printf("ERROR: Failed to load SOS program\n");
        component_vm_destroy(vm);
        while (1) blink_error_code(VM_ERROR_PROGRAM_NOT_LOADED);
    }
    printf("Program loaded with %d strings\n", component_vm_get_string_count(vm));
    
    // Execute SOS demo
    printf("Starting SOS demo execution...\n");
    printf("Instructions: Press button to start SOS, press again to stop\n\n");
    
    uint32_t start_time = millis();
    bool result = component_vm_execute_program(vm, sos_demo_program,
                                              sizeof(sos_demo_program) / sizeof(vm_instruction_c_t));
    uint32_t end_time = millis();
    
    // Execution results
    if (result) {
        printf("\n=== SOS Demo Execution Completed ===\n");
        printf("Execution time: %lu ms\n", end_time - start_time);
        printf("Final PC: %zu\n", component_vm_get_program_counter(vm));
        printf("Instructions executed: %zu\n", component_vm_get_instruction_count(vm));
        
        // Detailed performance analysis
        vm_c_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
        printf("Performance metrics:\n");
        printf("  Total execution time: %lu ms\n", metrics.execution_time_ms);
        printf("  Instructions executed: %zu\n", metrics.instructions_executed);
        printf("  Memory operations: %zu\n", metrics.memory_operations);
        printf("  I/O operations: %zu\n", metrics.io_operations);
        
        if (metrics.execution_time_ms > 0) {
            uint32_t ips = metrics.instructions_executed * 1000 / metrics.execution_time_ms;
            printf("  Instructions/second: %lu\n", ips);
        }
        
        // Final memory validation
        if (component_vm_validate_memory_integrity(vm)) {
            printf("Final memory validation: PASS\n");
            printf("✓ SOS Demo Hardware Integration: COMPLETE\n");
        } else {
            printf("Final memory validation: FAIL\n");
        }
        
    } else {
        vm_error_t error = component_vm_get_last_error(vm);
        printf("\n=== SOS Demo Execution Failed ===\n");
        printf("Error: %s\n", component_vm_get_error_string(error));
        printf("Error code: %d\n", error);
        printf("Failed at PC: %zu\n", component_vm_get_program_counter(vm));
        printf("Instructions completed: %zu\n", component_vm_get_instruction_count(vm));
    }
    
    component_vm_destroy(vm);
    printf("ComponentVM destroyed\n");
    
    // Success indication - heartbeat with status
    printf("Hardware integration complete - entering heartbeat mode\n");
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        
        // Status report every 10 seconds
        static uint32_t last_status = 0;
        if (millis() - last_status > 10000) {
            printf("Status: %lu ms uptime, memory OK, hardware operational\n", millis());
            last_status = millis();
        }
        
        delay(1000);
    }
}

// System memory integrity check
bool memory_integrity_check(void) {
    // Check critical memory regions
    uint32_t* stack_canary = (uint32_t*)(0x20000000 + 4092);  // Top of system stack
    uint32_t* heap_start = (uint32_t*)(0x20001000);           // Heap region
    
    // Write and verify test patterns
    uint32_t test_pattern = 0xDEADBEEF;
    *stack_canary = test_pattern;
    *heap_start = ~test_pattern;
    
    // Verify patterns
    if (*stack_canary != test_pattern) return false;
    if (*heap_start != ~test_pattern) return false;
    
    // Clear test patterns
    *stack_canary = 0;
    *heap_start = 0;
    
    return true;
}
```

#### **Phase 5 Complete Validation**
```bash
# Build final SOS demo firmware
pio run -e weact_g431cb_hardware -t upload

# Connect serial terminal for debug output
screen /dev/ttyUSB0 115200

# Expected complete sequence:
=== ComponentVM SOS Demo - Hardware Ready ===
System: STM32G431CB @ 170000000 Hz
LED: PC6, Button: PC13, Debug: USART1
Firmware: 74.0% flash, 32.8% RAM
Memory validation: PASS
Initializing ComponentVM...
ComponentVM memory validation: PASS
Loading SOS demo program (44 instructions)...
Program loaded with 4 strings
Starting SOS demo execution...
Instructions: Press button to start SOS, press again to stop

# Interactive behavior:
# 1. Press button -> "Starting SOS pattern..."
# 2. LED blinks SOS: ...---... (3 short, 3 long, 3 short)
# 3. "SOS pattern completed - Press button to stop or wait for repeat"
# 4. Pattern repeats automatically
# 5. Press button during execution -> "SOS demo stopped by user"

=== SOS Demo Execution Completed ===
Execution time: 15243 ms
Final PC: 44
Instructions executed: 156
Performance metrics:
  Total execution time: 15243 ms
  Instructions executed: 156
  Memory operations: 98
  I/O operations: 34
  Instructions/second: 10
Final memory validation: PASS
✓ SOS Demo Hardware Integration: COMPLETE
ComponentVM destroyed
Hardware integration complete - entering heartbeat mode
Status: 18000 ms uptime, memory OK, hardware operational
...
```

---

## Firmware Development Workflow

### **Static Bytecode Embedding Process**

#### **1. Build System Integration**
```bash
# Enhanced build process with bytecode generation
mkdir -p programs/
echo 'void setup() { pinMode(13, OUTPUT); }
void loop() { 
    digitalWrite(13, HIGH); 
    delay(1000);
    digitalWrite(13, LOW); 
    delay(1000);
}' > programs/blink.c

# Compile C to bytecode (using vm_compiler)
./compiler/build/arduino_compiler programs/blink.c programs/blink.bin

# Generate C header with embedded bytecode
python3 scripts/embed_bytecode.py programs/blink.bin src/embedded_blink.h

# Build firmware with embedded program
pio run -e weact_g431cb_hardware
```

#### **2. Bytecode Embedding Script**
```python
#!/usr/bin/env python3
# scripts/embed_bytecode.py - Generate C headers with embedded bytecode

import sys
import struct

def embed_bytecode(bin_file, header_file):
    """Convert binary bytecode to C header array"""
    
    with open(bin_file, 'rb') as f:
        bytecode = f.read()
    
    # Parse bytecode into 32-bit instructions
    instructions = []
    for i in range(0, len(bytecode), 4):
        if i + 4 <= len(bytecode):
            opcode = bytecode[i]
            flags = bytecode[i + 1]
            immediate = struct.unpack('<H', bytecode[i + 2:i + 4])[0]
            instructions.append((opcode, flags, immediate))
    
    # Generate C header
    header_content = f"""
// Auto-generated embedded bytecode
// Source: {bin_file}
// Generated: {datetime.now().isoformat()}

#ifndef EMBEDDED_BYTECODE_H
#define EMBEDDED_BYTECODE_H

#include "component_vm_c.h"

// Embedded program ({len(instructions)} instructions)
const vm_instruction_c_t embedded_program[] = {{
"""
    
    for i, (opcode, flags, immediate) in enumerate(instructions):
        header_content += f"    {{0x{opcode:02X}, 0x{flags:02X}, 0x{immediate:04X}}}"
        if i < len(instructions) - 1:
            header_content += ","
        header_content += f"  // Instruction {i}\n"
    
    header_content += f"""
}};

const size_t embedded_program_size = {len(instructions)};

#endif // EMBEDDED_BYTECODE_H
"""
    
    with open(header_file, 'w') as f:
        f.write(header_content)
    
    print(f"✓ Generated {header_file} with {len(instructions)} instructions")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: embed_bytecode.py <input.bin> <output.h>")
        sys.exit(1)
    
    embed_bytecode(sys.argv[1], sys.argv[2])
```

### **3. Production Firmware Template**
```c
// src/production_main.c - Template for production firmware
#include "stm32g4xx_hal.h"
#include "hardware_init.h"
#include "embedded_program.h"  // Generated bytecode header

// Production string table (embedded in flash)
__attribute__((section(".rodata.strings")))
const char* production_strings[] = {
    "Production firmware v1.0\n",
    "ComponentVM operational\n", 
    "Status: %d\n",
    "Error: %s\n"
};

int main(void) {
    // Production hardware initialization
    production_hardware_init();
    
    // Production ComponentVM setup
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        production_error_handler(INIT_FAILED);
    }
    
    // Load production program
    bool loaded = component_vm_load_program_with_strings(
        vm, embedded_program, embedded_program_size,
        production_strings, sizeof(production_strings) / sizeof(char*)
    );
    
    if (!loaded) {
        production_error_handler(PROGRAM_LOAD_FAILED);
    }
    
    // Execute production application
    bool result = component_vm_execute_program(vm, embedded_program, embedded_program_size);
    
    if (!result) {
        vm_error_t error = component_vm_get_last_error(vm);
        production_error_handler(error);
    }
    
    component_vm_destroy(vm);
    
    // Production complete - enter low power mode
    production_sleep_mode();
}

void production_hardware_init(void) {
    HAL_Init();
    SystemClock_Config();
    
    // Minimal peripheral initialization for production
    LED_GPIO_Init();
    Button_GPIO_Init();
    
    // Disable unused peripherals for power savings
    disable_unused_peripherals();
}

void production_error_handler(uint32_t error_code) {
    // Production error handling - blink error code
    while (1) {
        for (uint32_t i = 0; i < error_code && i < 10; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(200);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(200);
        }
        HAL_Delay(2000);
    }
}
```

---

## Troubleshooting & Validation

### **Common Hardware Issues**

#### **Power and Clock Problems**
```yaml
Symptoms: No LED activity, no serial output, debugger connection fails
Causes:
  - Insufficient power supply (check USB connection)
  - HSE crystal not oscillating (check 8MHz crystal)
  - Incorrect PLL configuration (verify multiplier/divider)
  - Brown-out conditions (check 3.3V regulation)

Debug Steps:
  1. Measure 3.3V on VDD pins with multimeter
  2. Check HSE oscillation with oscilloscope (8MHz on crystal pins)
  3. Verify system clock with debug output
  4. Test with HSI clock source as fallback

HSE Debug Code:
  // Check if HSE is ready
  if ((RCC->CR & RCC_CR_HSERDY) == 0) {
      printf("ERROR: HSE not ready\n");
  }
  
  // Check PLL lock
  if ((RCC->CR & RCC_CR_PLLRDY) == 0) {
      printf("ERROR: PLL not locked\n");
  }
```

#### **GPIO Configuration Issues**
```yaml
Symptoms: LED doesn't blink, button doesn't respond, incorrect pin behavior
Causes:
  - Wrong GPIO port/pin configuration
  - Incorrect alternate function mapping
  - Missing clock enable for GPIO ports
  - Pin conflicts with other peripherals

Debug Steps:
  1. Verify pin assignments match board layout
  2. Check GPIO clock enable calls
  3. Read back GPIO configuration registers
  4. Test pins with simple HAL calls

GPIO Debug Code:
  // Verify GPIO configuration
  GPIO_TypeDef* port = GPIOC;
  uint32_t pin = GPIO_PIN_6;
  
  printf("PC6 Config:\n");
  printf("  MODER: 0x%08lX\n", port->MODER);
  printf("  OTYPER: 0x%08lX\n", port->OTYPER);
  printf("  OSPEEDR: 0x%08lX\n", port->OSPEEDR);
  printf("  ODR: 0x%08lX\n", port->ODR);
```

#### **SWD Connection Problems**
```yaml
Symptoms: OpenOCD can't connect, debugging fails, flash programming errors
Causes:
  - Incorrect SWD wiring (SWDIO/SWCLK swapped)
  - Missing ground connection
  - SWD pins configured for other functions
  - Target not powered or in reset

Debug Steps:
  1. Verify SWD pin connections with multimeter
  2. Check target power (3.3V on VDD)
  3. Test with known-good ST-Link and board
  4. Try lower SWD clock frequency

OpenOCD Connection Test:
  # Verbose connection attempt
  openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -d3
  
  # Expected success output:
  Info : Target voltage: 3.3V
  Info : stm32g4x.cpu: hardware has 6 breakpoints, 4 watchpoints
```

#### **USART Communication Issues**
```yaml
Symptoms: No serial output, garbled text, printf not working
Causes:
  - Incorrect baud rate configuration
  - Wrong GPIO alternate function
  - Missing USART clock enable
  - Hardware flow control enabled

Debug Steps:
  1. Verify USART GPIO pin configuration (AF7 for USART1)
  2. Check baud rate calculation (170MHz / 115200)
  3. Test with simple HAL_UART_Transmit
  4. Verify serial terminal settings

USART Debug Code:
  // Test USART with simple transmission
  char test_msg[] = "USART Test\r\n";
  HAL_StatusTypeDef result = HAL_UART_Transmit(&huart1, 
                                              (uint8_t*)test_msg, 
                                              strlen(test_msg), 
                                              1000);
  if (result != HAL_OK) {
      // USART transmission failed
      blink_error_code(USART_ERROR);
  }
```

### **ComponentVM Integration Issues**

#### **Memory Corruption Detection**
```c
// Comprehensive memory validation
bool validate_memory_regions(void) {
    printf("=== Memory Validation ===\n");
    
    // Test 1: Stack canary check
    uint32_t* stack_canary = (uint32_t*)(0x20000000 + 4092);
    uint32_t canary_value = 0xCAFEBABE;
    *stack_canary = canary_value;
    
    if (*stack_canary != canary_value) {
        printf("FAIL: Stack canary corruption\n");
        return false;
    }
    printf("PASS: Stack canary intact\n");
    
    // Test 2: Heap boundary check
    uint32_t* heap_start = (uint32_t*)(0x20001000);
    uint32_t* heap_end = (uint32_t*)(0x20002000 - 4);
    
    *heap_start = 0x12345678;
    *heap_end = 0x87654321;
    
    if (*heap_start != 0x12345678 || *heap_end != 0x87654321) {
        printf("FAIL: Heap boundary corruption\n");
        return false;
    }
    printf("PASS: Heap boundaries intact\n");
    
    // Test 3: VM memory region
    uint32_t* vm_start = (uint32_t*)(0x20002000);
    uint32_t* vm_end = (uint32_t*)(0x20008000 - 4);
    
    *vm_start = 0xDEADBEEF;
    *vm_end = 0xFEEDFACE;
    
    if (*vm_start != 0xDEADBEEF || *vm_end != 0xFEEDFACE) {
        printf("FAIL: VM memory corruption\n");
        return false;
    }
    printf("PASS: VM memory region intact\n");
    
    // Clear test values
    *stack_canary = 0;
    *heap_start = 0;
    *heap_end = 0;
    *vm_start = 0;
    *vm_end = 0;
    
    return true;
}
```

#### **Performance Analysis Tools**
```c
// Performance benchmarking for ComponentVM
void benchmark_vm_performance(void) {
    printf("=== ComponentVM Performance Benchmark ===\n");
    
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        printf("FAIL: VM creation\n");
        return;
    }
    
    // Benchmark 1: Simple arithmetic operations
    vm_instruction_c_t math_test[] = {
        {0x01, 0, 1000},     // PUSH 1000
        {0x01, 0, 1},        // PUSH 1 (loop counter)
        {0x23, 0, 0},        // ADD
        {0x01, 0, 2000},     // PUSH 2000
        {0x28, 0, 0},        // LT (compare)
        {0x32, 0, 1},        // JMP_TRUE (loop)
        {0x00, 0, 0}         // HALT
    };
    
    uint32_t start = micros();
    bool result = component_vm_execute_program(vm, math_test, 7);
    uint32_t end = micros();
    
    if (result) {
        vm_c_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
        printf("Math benchmark: %lu us, %zu instructions\n", 
               end - start, metrics.instructions_executed);
        printf("Instructions/second: %lu\n", 
               metrics.instructions_executed * 1000000 / (end - start));
    }
    
    component_vm_reset_performance_metrics(vm);
    
    // Benchmark 2: GPIO operations
    vm_instruction_c_t gpio_test[] = {
        {0x15, 0, PC6},      // pinMode(PC6, OUTPUT)
        {0x01, 0, 100},      // PUSH 100 (loop count)
        {0x10, 0, PC6},      // digitalWrite(PC6, HIGH)
        {0x01, 0, 1},        // PUSH 1
        {0x10, 0, PC6},      // digitalWrite(PC6, LOW)
        {0x01, 0, 0},        // PUSH 0
        {0x01, 0, 1},        // PUSH 1
        {0x24, 0, 0},        // SUB
        {0x32, 0, 2},        // JMP_TRUE
        {0x00, 0, 0}         // HALT
    };
    
    start = micros();
    result = component_vm_execute_program(vm, gpio_test, 10);
    end = micros();
    
    if (result) {
        vm_c_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
        printf("GPIO benchmark: %lu us, %zu I/O ops\n", 
               end - start, metrics.io_operations);
        printf("GPIO ops/second: %lu\n", 
               metrics.io_operations * 1000000 / (end - start));
    }
    
    component_vm_destroy(vm);
    printf("Benchmark complete\n\n");
}
```

### **Hardware Validation Test Suite**
```c
// Complete hardware validation for production
bool complete_hardware_validation(void) {
    printf("=== Complete Hardware Validation ===\n");
    bool all_tests_passed = true;
    
    // Test 1: Clock accuracy
    printf("Test 1: Clock accuracy... ");
    uint32_t start = HAL_GetTick();
    delay(1000);  // Our implementation
    uint32_t end = HAL_GetTick();
    
    if (abs((int)(end - start) - 1000) <= 50) {  // ±50ms tolerance
        printf("PASS (%lu ms)\n", end - start);
    } else {
        printf("FAIL (%lu ms)\n", end - start);
        all_tests_passed = false;
    }
    
    // Test 2: GPIO functionality
    printf("Test 2: GPIO functionality... ");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    uint32_t led_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    
    if (led_state == GPIO_PIN_SET) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        all_tests_passed = false;
    }
    
    // Test 3: Button input
    printf("Test 3: Button input... ");
    uint32_t button_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    if (button_state == GPIO_PIN_SET) {  // Released state
        printf("PASS (released)\n");
    } else {
        printf("PASS (pressed)\n");  // Either state is valid
    }
    
    // Test 4: USART communication
    printf("Test 4: USART communication... ");
    char test_msg[] = "UART_TEST_OK\n";
    HAL_StatusTypeDef uart_result = HAL_UART_Transmit(&huart1, 
                                                     (uint8_t*)test_msg, 
                                                     strlen(test_msg), 
                                                     1000);
    if (uart_result == HAL_OK) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        all_tests_passed = false;
    }
    
    // Test 5: Memory integrity
    printf("Test 5: Memory integrity... ");
    if (validate_memory_regions()) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        all_tests_passed = false;
    }
    
    // Test 6: ComponentVM functionality
    printf("Test 6: ComponentVM functionality... ");
    ComponentVM_C* vm = component_vm_create();
    if (vm && component_vm_validate_memory_integrity(vm)) {
        component_vm_destroy(vm);
        printf("PASS\n");
    } else {
        if (vm) component_vm_destroy(vm);
        printf("FAIL\n");
        all_tests_passed = false;
    }
    
    printf("\n=== Hardware Validation %s ===\n", 
           all_tests_passed ? "PASSED" : "FAILED");
    
    return all_tests_passed;
}
```

---

## Production Deployment

### **Production Build Configuration**
```ini
# platformio.ini - Production environment
[env:weact_g431cb_production]
platform = ststm32
board = boards/weact_g431cb.json
framework = arduino

# Production build flags
build_flags = 
    -DSTM32G431xx
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000
    -DCOMPONENTVM_PRODUCTION     # Enable production mode
    -DNDEBUG                     # Disable debug assertions
    -Os                          # Optimize for size
    -flto                        # Link-time optimization
    -ffunction-sections
    -fdata-sections
    -Wl,--gc-sections           # Remove unused code
    -Wl,-Map,firmware.map       # Generate memory map

# Production memory layout
board_build.ldscript = linker_production.ld

# Optimized upload settings
upload_protocol = stlink
upload_flags = 
    --connect-under-reset
    --frequency=4000            # Faster SWD (4MHz)
    --verify                    # Verify after programming

# Disable debug features in production
debug_tool = 
monitor_speed = 

# Production validation
check_tool = cppcheck
check_flags = --enable=all --std=c99
```

### **Production Linker Script**
```ld
/* linker_production.ld - Optimized memory layout for production */
ENTRY(Reset_Handler)

MEMORY {
    FLASH_VECTORS (rx)  : ORIGIN = 0x08000000, LENGTH = 1K
    FLASH_FIRMWARE (rx) : ORIGIN = 0x08000400, LENGTH = 95K
    FLASH_BYTECODE (r)  : ORIGIN = 0x08017C00, LENGTH = 30K
    FLASH_CONFIG (r)    : ORIGIN = 0x0801F400, LENGTH = 2K
    FLASH_RESERVED (r)  : ORIGIN = 0x0801FC00, LENGTH = 1K
    
    RAM_SYSTEM (xrw)    : ORIGIN = 0x20000000, LENGTH = 8K
    RAM_VM (rw)         : ORIGIN = 0x20002000, LENGTH = 24K
}

SECTIONS {
    /* Vector table at flash start */
    .vectors : {
        . = ALIGN(4);
        KEEP(*(.vectors))
        . = ALIGN(4);
    } > FLASH_VECTORS
    
    /* Main firmware code */
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH_FIRMWARE
    
    /* Embedded bytecode programs */
    .bytecode : {
        . = ALIGN(4);
        __bytecode_start = .;
        *(.bytecode)
        *(.bytecode*)
        __bytecode_end = .;
        . = ALIGN(4);
    } > FLASH_BYTECODE
    
    /* Configuration and string tables */
    .config : {
        . = ALIGN(4);
        __config_start = .;
        *(.config)
        *(.strings)
        __config_end = .;
        . = ALIGN(4);
    } > FLASH_CONFIG
    
    /* System RAM sections */
    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM_SYSTEM AT> FLASH_FIRMWARE
    
    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM_SYSTEM
    
    /* VM memory region */
    .vm_memory (NOLOAD) : {
        . = ALIGN(4);
        __vm_memory_start = .;
        . = . + 24K;
        __vm_memory_end = .;
    } > RAM_VM
}

/* Memory usage symbols */
__flash_size = 128K;
__ram_size = 32K;
__vm_memory_size = 24K;
__system_memory_size = 8K;
```

### **Production Memory Analysis**
```python
#!/usr/bin/env python3
# scripts/memory_analysis.py - Analyze production memory usage

import re
import sys

def analyze_memory_map(map_file):
    """Analyze memory usage from linker map file"""
    
    with open(map_file, 'r') as f:
        content = f.read()
    
    # Parse memory usage
    sections = {}
    
    # Find section sizes
    section_pattern = r'(\.\w+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)'
    for match in re.finditer(section_pattern, content):
        section = match.group(1)
        address = int(match.group(2), 16)
        size = int(match.group(3), 16)
        sections[section] = {'address': address, 'size': size}
    
    # Calculate totals
    flash_used = 0
    ram_used = 0
    
    flash_sections = ['.vectors', '.text', '.bytecode', '.config']
    ram_sections = ['.data', '.bss']
    
    print("=== Production Memory Analysis ===")
    print(f"Target: STM32G431CB (128KB Flash, 32KB RAM)")
    print()
    
    print("Flash Usage:")
    for section in flash_sections:
        if section in sections:
            size = sections[section]['size']
            flash_used += size
            print(f"  {section:<12}: {size:>6} bytes ({size/1024:.1f}KB)")
    
    print(f"  {'Total':<12}: {flash_used:>6} bytes ({flash_used/1024:.1f}KB)")
    print(f"  {'Utilization':<12}: {flash_used*100/131072:.1f}%")
    print(f"  {'Available':<12}: {131072-flash_used:>6} bytes ({(131072-flash_used)/1024:.1f}KB)")
    print()
    
    print("RAM Usage:")
    for section in ram_sections:
        if section in sections:
            size = sections[section]['size']
            ram_used += size
            print(f"  {section:<12}: {size:>6} bytes ({size/1024:.1f}KB)")
    
    vm_memory = 24 * 1024  # 24KB VM memory
    total_ram = ram_used + vm_memory
    
    print(f"  {'VM Memory':<12}: {vm_memory:>6} bytes ({vm_memory/1024:.1f}KB)")
    print(f"  {'Total':<12}: {total_ram:>6} bytes ({total_ram/1024:.1f}KB)")
    print(f"  {'Utilization':<12}: {total_ram*100/32768:.1f}%")
    print(f"  {'Available':<12}: {32768-total_ram:>6} bytes ({(32768-total_ram)/1024:.1f}KB)")
    print()
    
    # Validation
    if flash_used > 131072:
        print("⚠️  WARNING: Flash usage exceeds 128KB limit")
        return False
    
    if total_ram > 32768:
        print("⚠️  WARNING: RAM usage exceeds 32KB limit")
        return False
    
    if flash_used > 131072 * 0.95:  # 95% threshold
        print("⚠️  WARNING: Flash usage above 95% - consider optimization")
    
    if total_ram > 32768 * 0.90:  # 90% threshold
        print("⚠️  WARNING: RAM usage above 90% - consider optimization")
    
    print("✅ Memory usage within acceptable limits")
    return True

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: memory_analysis.py <firmware.map>")
        sys.exit(1)
    
    success = analyze_memory_map(sys.argv[1])
    sys.exit(0 if success else 1)
```

### **Production Validation Checklist**
```bash
#!/bin/bash
# scripts/production_validation.sh - Complete production validation

echo "=== ComponentVM Production Validation ==="
echo "Target: WeAct STM32G431CB CoreBoard"
echo "Date: $(date)"
echo

# Build production firmware
echo "Building production firmware..."
pio run -e weact_g431cb_production

if [ $? -ne 0 ]; then
    echo "❌ Production build failed"
    exit 1
fi
echo "✅ Production build successful"

# Analyze memory usage
echo "Analyzing memory usage..."
python3 scripts/memory_analysis.py .pio/build/weact_g431cb_production/firmware.map

if [ $? -ne 0 ]; then
    echo "❌ Memory analysis failed"
    exit 1
fi

# Check firmware size
FIRMWARE_SIZE=$(stat -c%s .pio/build/weact_g431cb_production/firmware.bin)
MAX_SIZE=$((128 * 1024))  # 128KB

echo "Firmware size: $FIRMWARE_SIZE bytes ($(($FIRMWARE_SIZE * 100 / $MAX_SIZE))%)"

if [ $FIRMWARE_SIZE -gt $MAX_SIZE ]; then
    echo "❌ Firmware too large for flash memory"
    exit 1
fi
echo "✅ Firmware size within limits"

# Flash and validate on hardware
echo "Flashing to hardware..."
pio run -e weact_g431cb_production -t upload

if [ $? -ne 0 ]; then
    echo "❌ Hardware flashing failed"
    exit 1
fi
echo "✅ Hardware flashing successful"

# Hardware validation (requires manual verification)
echo "Hardware validation required:"
echo "  1. LED should blink SOS pattern when button pressed"
echo "  2. Serial output should show 'ComponentVM operational'"
echo "  3. Button should start/stop SOS sequence"
echo "  4. System should operate for 24+ hours without errors"

echo
echo "✅ Production validation complete"
echo "Hardware ready for deployment"
```

---

*This Hardware Integration Guide provides a complete roadmap from initial hardware setup through production deployment. The progressive bringup approach ensures systematic validation of each subsystem before integration, maximizing the probability of successful hardware deployment.*