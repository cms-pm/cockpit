# CockpitVM Project

[![Platform](https://img.shields.io/badge/Platform-STM32G474-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

**Research-Grade Embedded Virtual Machine for ARM Cortex-M4** - Complete end-to-end bytecode execution platform with ExecutionEngine_v2 featuring binary search dispatch, static memory allocation, and Arduino HAL integration. Proven guest ArduinoC → bytecode → STM32G4 hardware pipeline with comprehensive validation.

> **Phase 4.14 Complete** ✅ - End-to-end validation successful: ArduinoC guest programs executing on STM32G474 hardware through ComponentVM hypervisor with Oracle bootloader integration.

## Project Vision & Mission

**CockpitVM** is a research-grade embedded virtual machine enabling safe ArduinoC bytecode execution on ARM Cortex-M4 microcontrollers with deterministic performance, comprehensive hardware validation, with an aim to develop a better appreciation for modern embedded development by taking a hands-on, break things approach.

### **Core Achievements**
- **ExecutionEngine_v2** - Binary search dispatch with sparse jump table (112 opcodes, O(log n) performance)
- **HAL Integration** - Complete pinMode(), digitalWrite(), delay(), printf() support with hardware validation
- **Static Memory Architecture** - Per-VM memory isolation with VMMemoryContext (1.75KB per instance)
- **Golden Triangle Testing** - Comprehensive validation framework with stack verification and register inspection
- **Unified Error System** - Single source of truth error codes with bridge_c integration

## Current Status

### **Phase 4.14: End-to-End Validation Complete** ✅ **PROVEN**

**Complete Pipeline Validation:**
```
ArduinoC Source → vm_compiler → Bytecode → Oracle Bootloader → Page 63 Flash →
ComponentVM Auto-Execution → STM32G474 Hardware → Golden Triangle Validation
```

**Validation Report**: [Phase 4.14 End-to-End Validation Report](docs/development/qa/PHASE_4_14_END_TO_END_VALIDATION_REPORT.md)

**Key Achievements:**
- ✅ **Guest Program Execution** - ArduinoC blinky program executing on real STM32G474 hardware
- ✅ **Arduino HAL Validation** - pinMode(), digitalWrite(), delay() proven functional in guest context
- ✅ **Bytecode Auto-Execution** - Page 63 flash loading with magic signature (0x434F4E43 "CONC") and CRC-16 validation
- ✅ **Hardware Timing Accuracy** - 500ms delays accurate to ±2ms through platform-specific HAL integration
- ✅ **Oracle Bootloader Integration** - Dual-bank flash programming with protobuf protocol complete

### **Technical Highlights** ⚡

**From Validation Report**:

> **3.1 Test Configuration**
> - **Hardware**: STM32G474 WeAct CoreBoard @ 168MHz
> - **Flash Address**: Page 63 (0x0801F800, 2KB reserved)
> - **Guest Program**: blinky_basic.c (pinMode + digitalWrite + delay)
> - **Validation Method**: Hardware execution with timing measurement
>
> **3.2 Validation Results**
> - ✅ Magic signature validated (0x434F4E43 "CONC")
> - ✅ CRC-16-CCITT integrity check passed
> - ✅ String table scanned successfully (4 strings)
> - ✅ Guest bytecode auto-executed from flash
> - ✅ Hardware timing: 500ms delays measured at 498-502ms

**Critical Issues Resolved:**
1. **Delay Subsystem Failure** - Fixed `micros()` returning 0 causing infinite hang
2. **vm_compiler Timing Bug** - Implemented workaround for bogus ×17,000 multiplication
3. **Semihosting Platform Conflict** - UART routing prevents processor halt without active debugger
4. **String Table Corruption** - Dynamic pattern-based scanner bypasses vm_compiler header bugs

### **Current Development Roadmap**
- **Phase 4.15**: vm_compiler CVBC Compliance - Fix header generation, HALT opcode, delay multiplication bugs
- **Phase 5.0**: Cooperative Task Scheduler - Multi-program execution with static memory allocation
- **Ongoing**: Architecture refinement through experiential embedded development learning

## Technical Architecture

### **ExecutionEngine_v2: Binary Search Dispatch System**

CockpitVM achieves optimal performance through a sparse jump table with binary search opcode dispatch:

```cpp
// Sparse Jump Table with Binary Search (O(log n) dispatch)
struct OpcodeTableEntry {
    uint8_t opcode;
    vm_return_t (ExecutionEngine_v2::*handler)(uint16_t immediate) noexcept;
};

static constexpr OpcodeTableEntry OPCODE_TABLE[] = {
    // Sorted by opcode for binary search
    {0x00, &ExecutionEngine_v2::handle_halt_impl},
    {0x01, &ExecutionEngine_v2::handle_push_impl},
    {0x10, &ExecutionEngine_v2::handle_digital_write_impl},
    {0x11, &ExecutionEngine_v2::handle_digital_read_impl},
    {0x17, &ExecutionEngine_v2::handle_pin_mode_impl},
    // ... 112 total opcodes (0x00-0x6F)
};

// Binary search dispatch (cache-friendly, deterministic performance)
vm_return_t execute_instruction(uint8_t opcode, uint16_t immediate) {
    const auto* entry = std::lower_bound(OPCODE_TABLE, OPCODE_TABLE + SIZE, opcode);
    if (entry->opcode == opcode) {
        return (this->*entry->handler)(immediate);
    }
    return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
}
```

**Performance Benefits:**
- **O(log n) Dispatch**: 112 opcodes resolved in maximum 7 comparisons
- **Cache Efficiency**: Sequential memory access pattern for optimal performance
- **Deterministic Timing**: Predictable execution time for real-time systems

### **Hardware Platform**
```yaml
Target: STM32G474 WeAct Studio CoreBoard  
CPU: ARM Cortex-M4F @ 168MHz
Memory: 128KB Flash (dual-bank), 32KB RAM (static allocation)
Communication: USART1 Oracle bootloader client, USART2 Diagnostic Console
```

### **Memory Architecture: Static Allocation with Per-VM Isolation**

```cpp
// VMMemoryContext: 1.75KB per ComponentVM instance
struct VMMemoryContext {
    static constexpr size_t STACK_SIZE = 256;      // 1KB (256 * int32_t)
    static constexpr size_t GLOBAL_SIZE = 128;     // 512B (128 * int32_t)
    static constexpr size_t LOCAL_SIZE = 64;       // 256B (64 * int32_t)

    int32_t stack_[STACK_SIZE];      // Stack operations
    int32_t globals_[GLOBAL_SIZE];   // Global variables
    int32_t locals_[LOCAL_SIZE];     // Local variables

    size_t sp_;                      // Stack pointer
    // RAII cleanup, bounds checking, thread safety
};
```

**Memory Benefits:**
- **Static Allocation**: No heap fragmentation, deterministic memory usage
- **Per-VM Isolation**: Each ComponentVM instance has independent memory context
- **Bounds Checking**: All operations validate array indices for safety
- **RAII Cleanup**: Automatic memory management with predictable lifecycle

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

Current implementation achieves complete end-to-end validation of ArduinoC guest program execution on STM32G474 hardware. Phase 4.14 proves the complete pipeline from source code compilation through bytecode execution with hardware validation. The system demonstrates foundational embedded hypervisor concepts through practical tools:

- **Golden Triangle Framework** - Hardware validation with register inspection and semihosting
- **GT Lite Test Runner** - Microkernel testing with stack verification
- **Oracle Bootloader** - Dual-bank flash programming with protobuf protocol
- **ExecutionEngine_v2** - Binary search dispatch with 35+ opcode handlers
- **ComponentVM Auto-Execution** - Page 63 flash loading with integrity validation

The Phase 4.14 validation report documents the complete journey from initial execution hang to full operational success, demonstrating test-driven embedded development practices through experiential learning.

**Known Issues** (documented in [Validation Report](docs/development/qa/PHASE_4_14_END_TO_END_VALIDATION_REPORT.md)):
- vm_compiler header generation requires CVBC specification compliance
- vm_compiler emits RET (0x09) instead of HALT (0x00) - workaround in place
- vm_compiler delay multiplication by 17,000 - workaround divides by 17,000 in VM

---

For detailed information: [Phase 4.14 End-to-End Validation Report](docs/development/qa/PHASE_4_14_END_TO_END_VALIDATION_REPORT.md) • [ComponentVM Programmers Manual](docs/architecture/COMPONENTVM_PROGRAMMERS_MANUAL.md) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Architecture Documentation](docs/architecture/) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
