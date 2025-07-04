# 🚁 Cockpit Project

[![Embedded](https://img.shields.io/badge/Platform-Embedded-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]()

> **Lightweight embedded hypervisor for ARM Cortex-M4 microcontrollers**

Cockpit is an embedded hypervisor that provides a stack-based virtual machine environment for running bytecode programs on microcontrollers. Currently targeting ARM Cortex-M4 platforms with Arduino-compatible hardware abstraction, designed for QEMU-to-hardware transition.

## ✅ Current Implementation Status

**Phase 2.3.3 Complete** - Comparison Operations (EQ/NE/LT/GT/LE/GE) implemented and tested

### **🔧 Technical Achievements**

*   **🖥️ Virtual Machine Core:** 16-bit bytecode instruction set with stack-based operations (PUSH, POP, ADD, SUB, MUL, DIV, HALT)
*   **🔌 Arduino API Integration:** Complete Arduino functions (digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros) implemented as VM opcodes
*   **🎛️ Button Input System:** Debounced button handling with event queue and VM opcodes for press/release detection
*   **📄 Printf Implementation:** Format string support (%d, %s, %x, %c) with semihosting bridge for debug output
*   **🔍 Comparison Operations:** Full comparison opcode set (EQ/NE/LT/GT/LE/GE) with signed/unsigned variants and flags register
*   **⚙️ Hardware Abstraction Layer:** GPIO abstraction for ARM Cortex-M4 (Stellaris LM3S6965EVB) with comprehensive testing
*   **🧠 Memory Management:** Mocked 8KB RAM allocation with stack/heap separation, bounds checking, and overflow protection
*   **🖥️ QEMU Development Environment:** Automated testing with ARM semihosting, virtual time synchronization
*   **🏗️ Build System:** PlatformIO integration with automated compilation and reliable test execution

### **📊 Current Specifications (based on current emulated target specs)**

*   **💾 Flash Usage:** 20,200 bytes (15.4% of 128KB)
*   **🧮 RAM Usage:** 188 bytes static (0.9% of 20KB) + 8KB VM memory
*   **📝 Instruction Format:** 8-bit opcode + 8-bit immediate value
*   **🧪 Test Coverage:** 92 tests total, 100% pass rate
*   **🎯 Target Platform:** ARM Cortex-M4 with QEMU emulation
*   **🔧 VM Opcodes:** 15 Arduino functions + 6 core operations + printf + 12 comparison ops

### **🧪 Test Results (Latest)**
*   **VM Core Tests:** 21/21 passing (stack operations, arithmetic, memory)
*   **GPIO Tests:** 15/15 passing (digital I/O, HAL abstraction)
*   **Button Tests:** 20/20 passing (debouncing, event handling)
*   **Arduino Function Tests:** 36/36 passing (pinMode, timing, printf, comparisons)
*   **Overall Success Rate:** 100% (92/92 tests)

## 📈 MVP Development Roadmap

### **Phase 2: Arduino Integration** 🔄 **IN PROGRESS**
- ✅ **2.1** Arduino Digital GPIO Foundation (HAL + 5 Arduino functions)
- ✅ **2.2** Arduino Input + Button (debouncing, event queue, button opcodes)
- ✅ **2.3.1** pinMode + Timing Functions (pinMode, millis, micros opcodes)
- ✅ **2.3.2** printf() with Semihosting (%d %s %x %c formats)
- ✅ **2.3.3** Comparison Operations (EQ/NE/LT/GT/LE/GE opcodes)
- 🔄 **2.3.4** C-to-Bytecode Examples (Phase 3 preparation) - **CURRENT**
- 📋 **2.3.5** Documentation + Architecture Validation

### **Phase 3: C Compiler** 📋 **PLANNED**
- 📋 **3.1** Minimal C Parser Foundation (function parsing, variable handling)
- 📋 **3.2** Arduino Function Mapping (C calls to VM opcodes)
- 📋 **3.3** End-to-End Compilation Pipeline (C source to executable bytecode)
- ⚠️ **Mandatory**: 4+ Question/Answer planning cycles before implementation

### **Phase 4: Hardware Transition** 📋 **MVP TARGET**
- 📋 **4.1** Real Hardware Validation (STM32 or similar Cortex-M4)
- 📋 **4.2** Timing Precision Implementation (SysTick integration)
- 📋 **4.3** SOS Demo + Button Control (interactive demonstration)

**MVP Goal**: C-compiled bytecode running on real ARM Cortex-M4 hardware, demonstrating hypervisor independence from QEMU

## 🏗️ Architecture Overview

### **Bytecode Format**
```c
// 16-bit instructions: 8-bit opcode + 8-bit immediate
typedef struct {
    uint8_t opcode;     // Operation to perform
    uint8_t immediate;  // Pin number, value, or parameter
} vm_instruction_t;
```

### **Arduino API Integration**
```c
// Arduino functions mapped to VM opcodes
arduino_digital_write(PIN_13, PIN_HIGH);    // OP_DIGITAL_WRITE
arduino_digital_read(PIN_2);                // OP_DIGITAL_READ  
arduino_analog_write(PIN_13, 128);          // OP_ANALOG_WRITE
arduino_analog_read(0);                     // OP_ANALOG_READ
arduino_delay(1000);                        // OP_DELAY
```

### **VM Execution Example**
```c
// Bytecode program: Blink LED
uint16_t blink_program[] = {
    (OP_PUSH << 8) | 1,          // Push OUTPUT mode
    (OP_PIN_MODE << 8) | 13,     // pinMode(13, OUTPUT)
    (OP_PUSH << 8) | 1,          // Push HIGH state
    (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(13, HIGH)
    (OP_PUSH << 8) | 123,        // Push argument
    (OP_PUSH << 8) | 1,          // Push arg count
    (OP_PRINTF << 8) | 6,        // printf("Printf working: %d", 123)
    (OP_HALT << 8) | 0           // Stop execution
};
```

### **Memory Layout**
```
VM Memory (8KB):
┌─────────────┐ ← 0x20000000 + 8KB (stack_top)
│    Stack    │   4KB (grows downward)
│ (unused)    │
├─────────────┤ ← stack pointer (current)
│             │
│  Available  │
│             │
├─────────────┤ ← heap pointer (current)  
│    Heap     │   4KB (grows upward)
│  (program)  │
└─────────────┘ ← 0x20000000 (heap_base)
```

## 🛠️ Getting Started

### **Prerequisites**
*   PlatformIO CLI installed
*   QEMU ARM system emulation (for testing)
*   ARM GCC toolchain (managed by PlatformIO)

### **Building and Testing**
```bash
# Build the project
make build

# Run comprehensive test suite in QEMU
make test

# Interactive QEMU debugging session
make qemu

# Clean build artifacts
make clean
```

### **Expected Output**
```
Embedded Hypervisor MVP Starting...
=== VM Core Unit Tests Starting ===
...
=== ALL HYPERVISOR TESTS SUCCESSFUL ===
VM Core + GPIO + Button + Arduino Function tests passed
```

## 📁 Project Structure

```
cockpit/
├── src/                      # Main application and test code
│   ├── main.c               # Test orchestration and entry point
│   ├── test_vm_core.c       # VM core unit tests (21 tests)
│   ├── test_qemu_gpio.c     # QEMU GPIO tests (15 tests)  
│   ├── test_button_input.c  # Button system tests (20 tests)
│   └── test_arduino_functions.c # Arduino API tests (19 tests)
├── lib/                      # Custom libraries
│   ├── vm_core/             # Stack-based VM implementation
│   ├── arduino_hal/         # Arduino hardware abstraction
│   ├── button_input/        # Button debouncing system
│   └── semihosting/         # ARM semihosting for debug
├── scripts/                  # Build and test automation
│   └── qemu_runner.py       # QEMU execution and monitoring
├── docs/                     # Documentation
│   ├── VISION.md            # Long-term project vision
│   └── *.md                 # Implementation documentation
├── Makefile                  # Build automation
├── platformio.ini            # Build configuration
└── linker_script.ld          # Memory layout definition
```

## 🎯 Next Steps

### **Immediate (Phase 2.3.4)**
Create C-to-bytecode compilation examples and integration tests to prepare for Phase 3 C compiler implementation.

### **MVP Completion**
1. **Complete Phase 2.3**: Finish Arduino API integration with C-to-bytecode examples
2. **Plan Phase 3**: Systematic question/answer cycles for C compiler design  
3. **Implement C Compiler**: Hand-written minimal parser for Arduino function subset
4. **Hardware Validation**: Deploy to real ARM Cortex-M4 hardware
5. **Demo Creation**: Interactive SOS morse code with button control

**Target**: Demonstrate C-compiled bytecode running independently on ARM Cortex-M4 hardware

---

For long-term vision and advanced features, see [`docs/VISION.md`](docs/VISION.md).