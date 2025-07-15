# 🚁 Cockpit Project

[![Platform](https://img.shields.io/badge/Platform-STM32G431-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

> **Currently under heavy development - things will break - not for production use**

Embedded hypervisor project featuring ComponentVM - enabling C bytecode execution on ARM Cortex-M4 microcontrollers with Arduino-compatible hardware abstraction.

## 📋 Documentation

**Complete technical documentation: [docs/](docs/)**

- **[Getting Started](docs/GETTING_STARTED.md)** - Quick overview
- **[Architecture](docs/architecture/)** - System design (2000+ lines)  
- **[API Reference](docs/API_REFERENCE_COMPLETE.md)** - Function reference
- **[Hardware Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)** - STM32G431CB integration

---

## 🚀 Current Development Status

### **Phase 4.5.1: Hardware Validation System**
- **STM32G431CB Hardware**: Basic ARM Cortex-M4 @ 168MHz execution working
- **ComponentVM on Hardware**: Basic VM bytecode execution on hardware
- **Dual-Pass Validation**: Test framework for semihosting + hardware state validation
- **USART1 Testing**: Basic serial communication and register validation
- **Test Framework**: Workspace-isolated testing system
- **Clock Configuration**: 168MHz system clock + 48MHz USB clock configured

### **ComponentVM Core Features (Development)**
- **32-bit VM Architecture**: ARM Cortex-M4 implementation with PC management
- **Arduino Integration**: Basic digitalWrite, digitalRead, delay, pinMode, printf functions
- **C Compiler**: ANTLR4-based compiler with basic functions and control flow
- **Testing**: 181/181 tests passing on QEMU, basic hardware validation
- **Memory Protection**: Stack canaries, bounds checking, corruption detection

### **Hardware Configuration (STM32G431CB)**
```yaml
Platform: STM32G431CB WeAct Studio CoreBoard
CPU: ARM Cortex-M4F @ 168MHz
Flash: 12.5KB used (9.5% of 128KB)
RAM: 15KB used (46.2% of 32KB) - 8KB system + 24KB VM
VM Execution: Basic bytecode programs execute
USART1: Basic serial communication working
Test Framework: Development testing framework
Hardware Status: Development prototype
```

---

## 🎯 Phase 4 Progress: Hardware Transition

**Target**: STM32G431CB WeAct Studio CoreBoard

### **Completed Phases**
- ✅ **4.1 Hardware Foundation**: PlatformIO board, HAL adaptation, SysTick configuration
- ✅ **4.2 VM Integration**: Hardware HAL, C++ ComponentVM integration, VM Bridge layer
- ✅ **4.3 Automated Testing**: SWD test automation, GDB/OpenOCD integration
- ✅ **4.4 Button Validation**: Hardware input validation with register verification
- ✅ **4.5.1 Advanced Testing**: Dual-pass validation system, USART1 comprehensive testing

### **Current Phase**
- 🔄 **4.5.2 UART Bootloader**: UART transport foundation, command protocol, flash programming
  - **Strategy**: UART first (validated hardware), USB CDC as drop-in replacement
  - **Architecture**: Host Tool ←UART→ STM32 Bootloader (USB-Serial adapter)
  - **Modular Design**: Clean transport abstractions for easy USB CDC integration

### **Remaining Phases**
- ⏳ **4.5.3 Development Tools**: Host upload utility, deployment automation, end-to-end integration

---

## 🛠️ Quick Start

### **Prerequisites**
- PlatformIO CLI
- STM32G431CB board + ST-Link V2 debugger

### **Build & Test**
```bash
git clone <repository> && cd cockpit

# QEMU development (proven)
make build && make test

# Hardware execution (development)
~/.platformio/penv/bin/pio run --environment weact_g431cb_hardware
~/.platformio/penv/bin/pio run --target upload --environment weact_g431cb_hardware

# Hardware testing (Phase 4.5.1)
cd tests/
./setup_test_environment.sh
./tools/run_test pc6_led_focused        # GPIO with dual-pass validation
./tools/run_test usart1_comprehensive   # USART1 with register analysis
```

### **ComponentVM Example (Basic Hardware Execution)**
```c
// This C code compiles to bytecode and executes on STM32G431CB
void setup() {
    pinMode(13, OUTPUT);
    printf("ComponentVM on Hardware!\n");
}

void loop() {
    digitalWrite(13, HIGH);  // LED on
    delay(1000);             // Basic timing
    digitalWrite(13, LOW);   // LED off  
    delay(1000);             // Basic VM execution
}
```

**Hardware Testing**: VM programs execute with LED feedback + basic validation system

---

## 📊 Architecture

### **ComponentVM Architecture**
```
ExecutionEngine ←→ MemoryManager ←→ IOController
     ↓                   ↓                ↓
PC Management      Stack Canaries     Arduino HAL
Instruction        Bounds Checking    GPIO/Printf
Dispatch          Memory Protection   Timing
```

### **Memory Layout (Development)**
```
Flash (128KB): ComponentVM (12.5KB) + Bootloader (16KB planned) + Bytecode Storage (99KB available)
RAM (32KB): System Stack (8KB) + VM Memory (24KB)
Performance: Basic instruction execution, 1ms timing
Clock: 168MHz system + 48MHz USB (for future bootloader)
```

### **Instruction Format**
```c
typedef struct {
    uint8_t opcode;      // 256 operations
    uint8_t flags;       // Variants
    uint16_t immediate;  // Constants/addresses
} vm_instruction_t;
```

---

## 🏆 **Development Milestone**

ComponentVM now executes basic programs on STM32G431CB hardware. The transition from QEMU simulation to actual ARM Cortex-M4 silicon is underway, with basic VM functionality working on real hardware.

---

For detailed information: [Architecture Documentation](docs/architecture/) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)