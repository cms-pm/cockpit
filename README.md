# 🚁 CockpitVM Project

[![Platform](https://img.shields.io/badge/Platform-STM32G474-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

**Embedded Jail for ARM Cortex-M4** - Guest bytecode apps run under an embedded hypervisor, granted access to specific host peripherals via a generic API. Comprehensive testing system to support a TDD approach: compilation, execution, and memory are used to validate. Bytecode compiler based on ANTLR. Bootloader protocol to upload and verify guest bytecode. GUI to flash and verify.

> **Phase 4.8 Demo** - Multi-peripheral demo app to connect, test, and validate system as a whole.

## 🎯 Project Vision & Mission

**CockpitVM** is a research-grade embedded hypervisor enabling safe C bytecode execution on ARM Cortex-M4 microcontrollers with hardware-level safety, predictable performance, and multi-peripheral coordination.

### **Core Achievements**
- **32-bit Virtual Instruction Set** - Guest bytecode app runs within a VM with peripheral pass-thru to host
- **6-Layer Fresh Architecture** - Clean separation from Guest Application → Hardware  
- **Static Memory Allocation** - Compile-time task partitioning eliminates non-determinism
- **Peripheral Coordination** - In-progress: basic IO and display support
- **Serial Bootloader** - Wisdom of the Oracle bootloader client to support TDD combined with the Golden Triange test framework to validate compilation, execution, and memory

## 📊 Current Status

### **Phase 4.8: Demo Deployment** 🎯 **ACTIVE**
****
- **Core Peripherals**: OLED display, 5-button GPIO
- **Static Task Memory**: 4KB compile-time allocation (OLED 2.5KB, Button 1.25KB)
- **GUI Integration Tool**: CanopyUI provides flash upload, protocol execution post-mortem, and bytecode file info

### **Completed Milestones** ✅
- **Phase 4.6**: Oracle Bootloader Client Complete - Full protobuf bootloader cycle
- **Phase 4.7**: Host Bootloader Tool - Dual-bank flash programming implementation complete  
- **Phase 4.7.4**: Protocol Hardening - CRC16 validation + Universal Frame Parser divined by the Oracle

### **Upcoming Milestones**
- **Phase 4.9**: Update execution_engine - Instruction handler pattern optimized to memory bounds, fix compiler tests
- **Phase 5.0**: Preemptive RTOS Architecture - Multiple tasks with hardware timer coordination

## 🏗️ Technical Architecture

### **Hardware Platform**
```yaml
Target: STM32G474 WeAct Studio CoreBoard  
CPU: ARM Cortex-M4F @ 168MHz
Memory: 128KB Flash (dual-bank), 32KB RAM (static allocation)
Communication: USART1 Oracle bootloader client, USART2 Diagnostic Console
```

### **Memory Architecture (Static Allocation)**
```yaml
Flash: Bootloader (16KB) + Hypervisor (48KB) + Dual-Bank Bytecode (32KB each)
RAM (24KB VM): OLED (2.5KB) Button (1.25KB each) + Shared (512B)
Platform Controllers: GPIO + I2C + UART + Timer coordination
Resource Management: Mutex + reference counting
```

## 🛠️ Quick Start

### **Prerequisites**
- PlatformIO CLI + STM32G474 WeAct Studio CoreBoard + ST-Link V2
- Oracle python-based bootloader client: `/dev/ttyUSB2` + `tests/oracle_bootloader/oracle_venv`

### **Build & Deploy**
```bash
git clone <repository> && cd cockpit

# Hardware build and upload
~/.platformio/penv/bin/pio run --environment weact_g474_hardware --target upload

# Bootloader flash programming  
cd tests/oracle_bootloader && source oracle_venv/bin/activate
python oracle_cli.py --flash <bytecode_file>

# Multi-peripheral testing
cd tests && ./tools/run_test smp_sos_multimodal_coordination
```

---

## 📊 Architecture

### **Abstraction Layers**
```
Layer 6: Guest Application (Bytecode Programs)
         ↓
Layer 5: VM Hypervisor (CockpitVM Core)
         ↓  
Layer 4: Host Interface (gpio_pin_write, uart_begin)
         ↓
Layer 3: Platform Layer (STM32G4 adapter)
         ↓
Layer 2: STM32 HAL (Vendor library)
         ↓
Layer 1: Hardware (STM32G4)
```

### **Memory Layout (Research Implementation)**
```
Flash (128KB):
  Bootloader: 16KB     (CockpitVM bootloader)
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

CockpitVM Bootloader Framework implemented by grace of the Oracle bootloader test client, copying and verifying bytecode to flash memory. CanopyUI for external verification without direct SWD connection.

---

For detailed information: [Architecture Documentation](docs/architecture/) • [Integration Architecture Whitepaper](docs/COCKPITVM_INTEGRATION_ARCHITECTURE.md) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
