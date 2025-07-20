# 🚁 Cockpit Project

[![Platform](https://img.shields.io/badge/Platform-STM32G431-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

> **Currently under heavy development - things will break - not for production use**

Embedded hypervisor project featuring ComponentVM - enabling C bytecode execution on ARM Cortex-M4 microcontrollers with fresh embedded native architecture and cross-platform testing.

## 📋 Documentation

**Complete technical documentation: [docs/](docs/)**

- **[Getting Started](docs/GETTING_STARTED.md)** - Quick overview
- **[Architecture](docs/architecture/)** - System design (2000+ lines)  
- **[API Reference](docs/API_REFERENCE_COMPLETE.md)** - Function reference
- **[Hardware Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)** - STM32G431CB integration

---

## 🚀 Current Development Status

### **Fresh Architecture Complete (Phase 4.5.4)**
- **7-Layer Architecture**: Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware
- **Platform Test Interface**: Cross-platform hardware validation with STM32 HAL structures as single source of truth
- **Workspace-Isolated Testing**: Sophisticated test system with comprehensive hardware validation
- **STM32G431CB Production Ready**: Clean layer boundaries, reliable hardware operation
- **Embedded Native API**: Professional gpio_pin_write, uart_begin interface (Arduino compatibility removed)

### **ComponentVM Core Features (Production Ready)**
- **32-bit VM Architecture**: ARM Cortex-M4 implementation with explicit PC management
- **Host Interface API**: gpio_pin_write, uart_begin, delay_ms - embedded native naming
- **C Compiler**: ANTLR4-based compiler with functions, control flow, expressions
- **Testing**: 100% pass rate on hardware with platform test interface validation
- **Memory Protection**: Stack canaries, bounds checking, comprehensive corruption detection

### **Hardware Configuration (STM32G431CB)**
```yaml
Platform: STM32G431CB WeAct Studio CoreBoard
CPU: ARM Cortex-M4F @ 168MHz system, 48MHz USB
Memory: 128KB Flash, 32KB RAM
Peripherals: GPIO (PC6 LED), USART1 (PA9/PA10), SWD debug
VM Execution: Full bytecode programs with host interface API
Testing: Platform test interface with cross-platform validation
Architecture: 7-layer fresh architecture with clean boundaries
Hardware Status: Production-ready with comprehensive validation
```

---

## 🎯 Phase 4 Progress: Hardware Transition

**Target**: STM32G431CB WeAct Studio CoreBoard

### **Completed Phases**
- ✅ **4.1 Hardware Foundation**: PlatformIO board, HAL adaptation, embedded native API
- ✅ **4.2 VM Integration**: Fresh architecture implementation, clean layer boundaries
- ✅ **4.3 Automated Testing**: Workspace-isolated testing with OpenOCD/GDB integration
- ✅ **4.5.4 Fresh Architecture**: Complete 7-layer architecture with platform test interface
  - **Platform Test Interface**: STM32 HAL structures as single source of truth
  - **Cross-Platform Testing**: Same test logic, platform-specific validation
  - **Workspace Template Enhancement**: Platform-aware workspace generation
  - **Hardware Validation**: Comprehensive testing on real STM32G431CB hardware

### **Current Phase**
- 🎯 **4.5.2 Bootloader System**: Production-ready bootloader with dual-bank strategy
  - **Transport Abstraction**: UART first, USB CDC drop-in capability
  - **Dual-Bank Architecture**: Atomic bytecode updates with rollback capability
  - **Production Reliability**: Hierarchical error states, overflow-safe timeouts, resource cleanup
  - **Complements STM32 Bootloader**: Bytecode updates vs firmware updates
  - **Documentation**: [Bootloader Design](docs/hardware/phase-4/PHASE_4_5_2_BOOTLOADER_DESIGN.md)

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

# Hardware execution (production ready)
python scripts/switch_target.py hardware
~/.platformio/penv/bin/pio run --target upload

# Workspace-isolated testing with platform test interface
cd tests/
./setup_test_environment.sh
./tools/run_test pc6_led_focused           # GPIO with sophisticated debugging
./tools/run_test usart1_comprehensive      # UART with platform interface validation
./tools/run_test vm_arithmetic_comprehensive # VM operations validation
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

### **Fresh Architecture (7-Layer)**
```
Layer 7: Guest Application (Bytecode Programs)
         ↓
Layer 6: VM Hypervisor (ComponentVM Core)
         ↓  
Layer 5: Host Interface (gpio_pin_write, uart_begin)
         ↓
Layer 4: Platform Layer (STM32G4 adapter)
         ↓
Layer 3: STM32 HAL (Vendor library)
         ↓
Layer 2: Hardware (STM32G431CB)
```

### **Memory Layout (Production)**
```
Flash (128KB):
  Bootloader: 16KB     (ComponentVM bootloader)
  Hypervisor: 48KB     (VM runtime + host interface)
  Bytecode Bank A: 32KB (Active bytecode)
  Bytecode Bank B: 32KB (Receive/backup bytecode)

RAM (32KB):
  System: 8KB          (bootloader + hypervisor)
  VM Memory: 24KB      (guest applications)

Clock: 168MHz system + 48MHz USB (validated)
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

ComponentVM fresh architecture complete with production-ready 7-layer design. Platform test interface enables cross-platform testing with STM32 HAL structures as single source of truth. Ready for bootloader implementation.

---

For detailed information: [Architecture Documentation](docs/architecture/) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)