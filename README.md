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

## 🚀 Current Status: Phase 3 Complete

### **ComponentVM Achievements**
- ✅ **32-bit VM Architecture**: ARM Cortex-M4 optimized with HandlerReturn PC management
- ✅ **Arduino Integration**: digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros, printf
- ✅ **C Compiler**: ANTLR4-based with functions, control flow, expressions
- ✅ **Testing**: 181/181 tests passing, QEMU development environment
- ✅ **Memory Protection**: Stack canaries, bounds checking

### **Technical Specs**
```yaml
Target: STM32G431CB (ARM Cortex-M4 @ 170MHz)
Flash: 97KB used (76% of 128KB)
RAM: 32KB (8KB system + 24KB VM)
Tests: 181/181 passing (100%)
Opcodes: 80+ in semantic groups
```

---

## 🎯 Phase 4: Hardware Transition

**Target**: STM32G431CB WeAct Studio CoreBoard

### **Development Plan**
- **4.1 Hardware Foundation**: PlatformIO board, HAL adaptation
- **4.2 VM Integration**: Hardware HAL, system interfaces  
- **4.3 Validation**: SWD testing, performance benchmarks
- **4.4 Bootloader**: UART protocol, flash programming
- **4.5 Tools**: Bytecode upload, end-to-end testing

---

## 🛠️ Quick Start

### **Prerequisites**
- PlatformIO CLI
- STM32G431CB board + ST-Link V2 debugger

### **Build & Test**
```bash
git clone <repository> && cd cockpit

# QEMU development
make build && make test

# Hardware (Phase 4)
pio run --environment stm32g431cb_dev
pio run --target upload
```

### **Example**
```c
void setup() {
    pinMode(13, OUTPUT);
    printf("Starting\n");
}

void loop() {
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
}
```

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

### **Memory Layout**
```
Flash (128KB): Firmware (96KB) + Bytecode (30KB)
RAM (32KB): System (8KB) + VM Memory (24KB)
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

For detailed information: [Architecture Documentation](docs/architecture/) • [API Reference](docs/API_REFERENCE_COMPLETE.md)