# 🚁 CockpitVM Project

[![Platform](https://img.shields.io/badge/Platform-STM32G474-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

**Embedded Jail for ARM Cortex-M4** - Guest bytecode apps run under an embedded hypervisor, granted access to specific host peripherals via a generic API. Comprehensive testing system to support a TDD approach: compilation, execution, and memory are used to validate. Bytecode compiler based on ANTLR. Bootloader protocol to upload and verify guest bytecode. GUI to flash and verify.

> **Phase 4.8 Demo** - Multi-peripheral demo app to connect, test, and validate system as a whole.

## 🎯 Project Vision & Mission

**CockpitVM** is a research-grade embedded hypervisor enabling safe C bytecode execution on ARM Cortex-M4 microcontrollers with hardware-level safety, predictable performance, and multi-peripheral coordination.

### **Core Research Areas**
- **32-bit Virtual Instruction Set** - Stack-based VM with peripheral isolation and validation
- **6-Layer Architecture** - Clean abstraction layers from Guest Application → Hardware  
- **Static Memory Allocation** - Research into compile-time task partitioning for determinism
- **Trinity Zero-Cost Abstraction** - Research implementation of three-tier hardware abstraction templates
- **Serial Bootloader Protocol** - Oracle bootloader client supporting test-driven development methodology

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

### **Research Development Roadmap**
- **Phase 4.9**: Trinity Architecture Implementation - Three-tier zero-cost hardware abstraction system
- **Phase 5.0**: Cooperative Task Scheduler - Multi-program switching with static memory allocation  
- **Future Research**: RTOS integration, cross-platform template expansion, security enhancements
- **Tooling**: CanopyUI standalone repository, enhanced Oracle bootloader protocol

## 🏗️ Technical Architecture

### **Trinity: Zero-Cost Hardware Abstraction (Phase 4.9 Research)**

CockpitVM is developing **Trinity** - a revolutionary three-tier template system achieving bare-metal performance with hardware independence:

```
Tier 1: Template Hardware Descriptors (90% operations - zero runtime cost)
Tier 2: Runtime HAL Integration     (9% operations - performance fallback)  
Tier 3: Generic Register Interface  (1% edge cases - compatibility layer)
```

**Research Goal**: `digitalWrite(13, HIGH)` compiles to **single instruction**: `GPIOC->BSRR = GPIO_PIN_6`

**Documentation**: [Trinity Architecture](docs/architecture/ZERO_COST_HARDWARE_ABSTRACTION_ARCHITECTURE.md) • [CVBC Format](docs/technical/CVBC_BYTECODE_FORMAT_SPECIFICATION.md) • [Implementation Plan](docs/development/PHASE_4_9_ZERO_COST_IMPLEMENTATION_PLAN.md)

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

### **Memory Layout (Current Research Implementation)**
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

## 🔬 **Research Status**

Current implementation focuses on foundational embedded hypervisor concepts with the Oracle bootloader protocol enabling test-driven development. Trinity architecture represents ongoing research into zero-cost abstraction techniques for embedded systems.

---

For detailed information: [Architecture Documentation](docs/architecture/) • [Integration Architecture Whitepaper](docs/COCKPITVM_INTEGRATION_ARCHITECTURE.md) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
