# CockpitVM Project

[![Platform](https://img.shields.io/badge/Platform-STM32G474-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

**Research-Grade Embedded Virtual Machine for ARM Cortex-M4** - Advanced bytecode execution platform with ExecutionEngine_v2 featuring binary search dispatch, static memory allocation, and guest HAL integration. Supports guest ArduinoC programs with deterministic real-time execution and Golden Triangle hardware validation.

> **Phase 4.13 Complete** - ExecutionEngine_v2 with 35+ opcode handlers and 9/9 HAL operations validated.

## Project Vision & Mission

**CockpitVM** is a research-grade embedded virtual machine enabling safe ArduinoC bytecode execution on ARM Cortex-M4 microcontrollers with deterministic performance, comprehensive hardware validation, with an aim to develop a better appreciation for modern embedded development by taking a hands-on, break things approach.

### **Core Achievements**
- **ExecutionEngine_v2** - Binary search dispatch with sparse jump table (112 opcodes, O(log n) performance)
- **Arduino HAL Integration** - Complete pinMode(), digitalWrite(), delay(), printf() support with hardware validation
- **Static Memory Architecture** - Per-VM memory isolation with VMMemoryContext (1.75KB per instance)
- **Golden Triangle Testing** - Comprehensive validation framework with stack verification and register inspection
- **Unified Error System** - Single source of truth error codes with bridge_c integration

## Current Status

### **Phase 4.13: ExecutionEngine_v2 Complete** ✅ **DELIVERED**

**Major Architectural Achievements:**
- **9/9 Arduino HAL Operations** - pinMode, digitalWrite, digitalRead, analogWrite, analogRead, delay, millis, micros, printf
- **35+ Opcode Handlers** - Control flow, logical operations, memory operations, and Arduino HAL integration
- **Binary Search Dispatch** - O(log n) opcode lookup with sparse jump table for 112 total opcodes
- **QEMU_PLATFORM Testing** - Comprehensive mock implementations for validation
- **Stack Verification Framework** - GT Lite enhanced with actual stack content validation

### **Critical Bugs Eliminated** ✅
- **Dual-Dispatch Infinite Recursion** - ExecutionEngine_v2 eliminates recursive handler calls
- **Duplicate Opcode Definitions** - Single source of truth in vm_opcodes.h (112 opcodes, 0x00-0x6F)
- **Memory Management Architecture** - Clean VMMemoryContext isolation per ComponentVM instance
- **Error Code Conflicts** - Unified vm_error_t system across ExecutionEngine_v2 and bridge_c

### **Current Development Roadmap**
- **Phase 4.14**: End-to-End Demo - Guest ArduinoC → CockpitVM → STM32G474 hardware validation
- **Phase 5.0**: Cooperative Task Scheduler - Multi-program execution with static memory allocation
- **Learning by Doing**: Architecture suitable for embedded experiments with deterministic performance

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
class VMMemoryContext {
private:
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

Current implementation focuses on foundational embedded hypervisor concepts with the Golden Triangle test framework, the GT Lite microkernel test runner, and the Oracle bootloader flash client enabling test-driven development. ExecutionEngine_v2 represents a step toward realising a deeper understanding of embedded system
design and testing practices.

---

For detailed information: [ComponentVM Programmers Manual](docs/architecture/COMPONENTVM_PROGRAMMERS_MANUAL.md) • [Architecture Documentation](docs/architecture/) • [Integration Architecture Whitepaper](docs/COCKPITVM_INTEGRATION_ARCHITECTURE.md) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
