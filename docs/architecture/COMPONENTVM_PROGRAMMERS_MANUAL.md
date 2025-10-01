# ComponentVM Programmer's Manual

**CockpitVM Architecture Documentation**
**Executive Summary**: Complete guide to ComponentVM's ExecutionEngine_v2, MemoryManager, and IOController integration

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [ExecutionEngine_v2 Design](#executionengine_v2-design)
3. [Memory Management Architecture](#memory-management-architecture)
4. [IOController Integration](#iocontroller-integration)
5. [Component Interaction Model](#component-interaction-model)

---

## 1. Architecture Overview

ComponentVM implements a naive OSI-inspired architecture for embedded virtual machine execution on STM32G474 hardware:

```
┌─────────────────────────────────────────┐
│           Guest Application             │ ← ArduinoC bytecode
├─────────────────────────────────────────┤
│            VM Hypervisor               │ ← ComponentVM + ExecutionEngine_v2
├─────────────────────────────────────────┤
│           Host Interface               │ ← Memory + IO abstraction
├─────────────────────────────────────────┤
│          Platform Layer                │ ← STM32G4 HAL integration
├─────────────────────────────────────────┤
│            STM32 HAL                   │ ← Hardware abstraction
├─────────────────────────────────────────┤
│            Hardware                    │ ← STM32G474 WeAct CoreBoard
└─────────────────────────────────────────┘
```

### Core Components

**ComponentVM** acts as the orchestrator, managing:
- **ExecutionEngine_v2**: Sparse jump table bytecode execution with binary search dispatch
- **VMMemoryContext**: Static 24KB memory allocation with per-VM isolation
- **MemoryManager**: Legacy interface bridge to VMMemoryContext
- **IOController**: Arduino HAL abstraction with automatic debugger detection

---

## 2. ExecutionEngine_v2 Design

### 2.1 Sparse Jump Table Architecture

ExecutionEngine_v2 implements a binary search dispatch system for optimal performance:

```cpp
struct OpcodeTableEntry {
    uint8_t opcode;
    vm_return_t (ExecutionEngine_v2::*handler)(uint16_t immediate) noexcept;
};

static constexpr OpcodeTableEntry OPCODE_TABLE[] = {
    // Sorted by opcode for binary search
    {0x00, &ExecutionEngine_v2::handle_halt_impl},
    {0x01, &ExecutionEngine_v2::handle_push_impl},
    {0x10, &ExecutionEngine_v2::handle_digital_write_impl},
    // ... 112 total opcodes (0x00-0x6F)
};

// Binary search dispatch (O(log n), cache-friendly)
ExecutionEngine_v2::HandlerMethod get_handler(uint8_t opcode) {
    const opcode_handler_entry* entry = std::lower_bound(
        OPCODE_TABLE,
        OPCODE_TABLE + OPCODE_TABLE_SIZE,
        opcode,
        [](const opcode_handler_entry& e, uint8_t op) { return e.opcode < op; }
    );

    if (entry < OPCODE_TABLE + OPCODE_TABLE_SIZE && entry->opcode == opcode) {
        return entry->handler;
    }
    return &ExecutionEngine_v2::handle_invalid_opcode_impl;
}
```

### 2.2 Handler Call Graph

```
execute_single_instruction()
    ├── binary_search_handler(opcode)
    ├── (this->*handler)(immediate)
    │   ├── Arduino HAL handlers
    │   │   ├── handle_digital_write_impl()
    │   │   │   ├── pop_protected(value)
    │   │   │   ├── pop_protected(pin)
    │   │   │   └── io_->digital_write(pin, value)
    │   │   ├── handle_pin_mode_impl()
    │   │   └── handle_millis_impl()
    │   ├── Memory handlers
    │   │   ├── handle_load_global_impl()
    │   │   └── handle_store_global_impl()
    │   └── Control flow handlers
    │       ├── handle_jmp_impl()
    │       └── handle_jmp_true_impl()
    └── update_pc_based_on_result()
```

### 2.3 vm_return_t State Management

ExecutionEngine_v2 uses a unified result type for instruction control flow:

```cpp
struct vm_return_t {
    enum Type { SUCCESS, ERROR, JUMP };

    static vm_return_t success() noexcept;
    static vm_return_t error(vm_error_t error) noexcept;
    static vm_return_t jump(size_t target_pc) noexcept;
};
```

**Benefits**:
- Single return path eliminates dual-dispatch recursion
- Jump target semantics use instruction indices (not byte offsets)
- Error propagation preserves original vm_error_t codes

---

## 3. Memory Management Architecture

### 3.1 VMMemoryContext: Static Allocation Model

VMMemoryContext provides deterministic memory allocation for real-time embedded execution:

```cpp
struct VMMemoryContext {
    static constexpr size_t STACK_SIZE = 256;      // 1KB (256 * 4 bytes)
    static constexpr size_t GLOBAL_SIZE = 128;     // 512B (128 * 4 bytes)
    static constexpr size_t LOCAL_SIZE = 64;       // 256B (64 * 4 bytes)

    int32_t stack_[STACK_SIZE];
    int32_t globals_[GLOBAL_SIZE];
    int32_t locals_[LOCAL_SIZE];

    size_t sp_;           // Stack pointer
    size_t global_count_; // Global variable count
    size_t local_count_;  // Local variable count
};
```

### 3.2 Memory Layout per VM Instance

Each ComponentVM instance gets its own isolated memory context:

```
VM Instance Memory Layout (1.75KB total per VM)
┌─────────────────────────────────────────┐
│              Stack (1KB)                │ ← 256 * int32_t
│  sp_ →  [value_n] [value_n-1] ... [0]   │
├─────────────────────────────────────────┤
│            Globals (512B)               │ ← 128 * int32_t
│  [global_0] [global_1] ... [global_127] │
├─────────────────────────────────────────┤
│             Locals (256B)               │ ← 64 * int32_t
│  [local_0] [local_1] ... [local_63]     │
└─────────────────────────────────────────┘
```

### 3.3 Memory Operation Call Chain

```
ExecutionEngine_v2::handle_load_global_impl()
    ├── memory_.load_global_protected(index, value)
    │   └── VMMemoryContext::load_global(index, value)
    │       ├── bounds_check(index < global_count_)
    │       └── value = globals_[index]
    └── push_protected(value)
        └── VMMemoryContext::push(value)
            ├── bounds_check(sp_ < STACK_SIZE)
            └── stack_[sp_++] = value
```

### 3.4 Thread Safety & RAII

- **Static Allocation**: No heap allocation eliminates fragmentation
- **Bounds Checking**: All operations validate array indices
- **RAII-secured Cleanup**: VMMemoryContext::reset() called in ComponentVM destructor 
- **No Shared State**: Each VM instance has isolated memory

---

## 4. IOController Integration

### 4.1 Arduino HAL Abstraction

IOController provides a hardware-agnostic interface for guest bytecode:

```cpp
class IOController {
public:
    // Arduino-compatible digital I/O
    bool digital_write(uint8_t pin, uint8_t value) noexcept;
    bool digital_read(uint8_t pin, uint8_t& value) noexcept;
    bool pin_mode(uint8_t pin, uint8_t mode) noexcept;

    // Arduino-compatible analog I/O
    bool analog_write(uint8_t pin, uint16_t value) noexcept;
    bool analog_read(uint8_t pin, uint16_t& value) noexcept;

    // Timing functions
    uint32_t millis() const noexcept;
    uint32_t micros() const noexcept;
    void delay(uint32_t ms) noexcept;
};
```

### 4.2 Platform Abstraction Layers

IOController implements conditional compilation for multiple targets:

```cpp
bool IOController::hal_digital_write(uint8_t pin, uint8_t value) noexcept {
    #ifdef ARDUINO_PLATFORM
    digitalWrite(pin, value);
    return true;
    #elif defined(QEMU_PLATFORM)
    char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg),
             "Digital write: pin %d = %d\n", pin, value);
    route_printf(debug_msg);
    return true;
    #elif defined(PLATFORM_STM32G4)
    return platform_gpio_write(pin, value);
    #endif
}
```

### 4.3 Automatic Printf Routing

IOController detects runtime environment and routes printf appropriately:

```cpp
void IOController::route_printf(const char* message) noexcept {
    #ifdef PLATFORM_STM32G4
    if (is_debugger_connected()) {
        semihost_write_string(message);  // CoreDebug interface
    } else {
        usart2_write_string(message);    // USART2 PA2/PA3@115200
    }
    #elif defined(QEMU_PLATFORM)
    printf("%s", message);               // Host stdout
    #endif
}
```

---

## 5. Component Interaction Model

### 5.1 ComponentVM Orchestration

ComponentVM coordinates all subsystems through RAII and dependency injection:

```cpp
class ComponentVM {
private:
    #ifdef USE_EXECUTION_ENGINE_V2
    ExecutionEngine_v2 engine_;          // Constructed first
    #else
    ExecutionEngine engine_;             // Legacy fallback
    #endif
    MemoryManager memory_;               // Static pool manager
    IOController io_;                    // Interfacing with hardware

public:
    bool execute_program(const VM::Instruction* program, size_t program_size) noexcept {
        // Single-step execution with observer notifications
        while (!engine_.is_halted() && instruction_count_ < program_size) {
            uint32_t pc = static_cast<uint32_t>(engine_.get_pc());

            // Extract instruction data for telemetry
            const VM::Instruction& current_instr = program[pc];
            uint8_t opcode = current_instr.opcode;
            uint32_t operand = static_cast<uint32_t>(current_instr.immediate);

            // Execute with direct component access
            if (!engine_.execute_single_instruction(memory_, io_)) {
                vm_error_t error = engine_.get_last_error();
                set_error(error);
                return false;
            }

            // Notify observers with real instruction data
            notify_instruction_executed(pc, opcode, operand);
            instruction_count_++;
        }
        return true;
    }
};
```

### 5.2 Execution Flow Diagram

```
┌─────────────────┐    load_program()    ┌─────────────────┐
│   Guest Code    │────────────────────→│   ComponentVM   │
│   (Bytecode)    │                     │                 │
└─────────────────┘                     └─────────────────┘
                                                  │
                                        execute_program()
                                                  │
                                                  ▼
                        ┌─────────────────────────────────────────────┐
                        │          Execution Loop                     │
                        │  ┌─────────────────────────────────────┐   │
                        │  │     ExecutionEngine_v2              │   │
                        │  │  ┌─────────────────────────────┐    │   │
                        │  │  │   Binary Search Dispatch   │    │   │
                        │  │  │   handle_*_impl(immediate) │    │   │
                        │  │  └─────────────────────────────┘    │   │
                        │  │             │         │             │   │
                        │  │             ▼         ▼             │   │
                        │  │    ┌─────────────┐ ┌──────────────┐ │   │
                        │  │    │MemoryManager│ │ IOController │ │   │
                        │  │    │push/pop     │ │digital_write │ │   │
                        │  │    │load/store   │ │pin_mode      │ │   │
                        │  │    └─────────────┘ └──────────────┘ │   │
                        │  └─────────────────────────────────────┘   │
                        │                    │                       │
                        │         ┌─────────────────────┐            │
                        │         │  Observer Pattern   │            │
                        │         │  notify_instruction │            │
                        │         │  _executed()        │            │
                        │         └─────────────────────┘            │
                        └─────────────────────────────────────────────┘
```

### 5.3 Error Propagation Model

Unified error handling flows from ExecutionEngine_v2 through ComponentVM:

```cpp
// ExecutionEngine_v2 handler error
vm_return_t ExecutionEngine_v2::handle_digital_write_impl(uint16_t immediate) {
    if (!io_->digital_write(pin, value)) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);  // Unified error code
    }
    return vm_return_t::success();
}

// ComponentVM error propagation
bool ComponentVM::execute_single_step() {
    if (!engine_.execute_single_instruction(memory_, io_)) {
        vm_error_t engine_error = engine_.get_last_error();  // Extract error
        set_error(engine_error);                             // Propagate to ComponentVM
        return false;
    }
    return true;
}

// Bridge_c interface error handling
bool enhanced_vm_execute_with_diagnostics(enhanced_vm_context_t* ctx) {
    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);
    bool success = vm->execute_program(program, program_size);
    if (!success) {
        vm_error_t error = vm->get_last_error();  // Available for diagnostics
        // Error propagates to GT Lite test framework
    }
    return success;
}
```

### 5.4 Memory Isolation Guarantees

Each ComponentVM instance provides complete memory isolation:

```cpp
// Multiple VM instances with isolated memory
ComponentVM vm1;  // Gets own VMMemoryContext (1.75KB)
ComponentVM vm2;  // Gets own VMMemoryContext (1.75KB)
ComponentVM vm3;  // Gets own VMMemoryContext (1.75KB)

// No shared state between VMs
vm1.execute_program(bytecode_a, size_a);  // Isolated execution
vm2.execute_program(bytecode_b, size_b);  // Isolated execution
vm3.execute_program(bytecode_c, size_c);  // Isolated execution

// Each VM has independent:
// - Stack pointer and contents
// - Global variable storage
// - Local variable storage
// - Program counter state
// - Error state
```

---

## Conclusion

ComponentVM's ExecutionEngine_v2 architecture provides:

✅ **Performance**: Binary search dispatch + sparse jump table
✅ **Safety**: Static memory allocation + bounds checking
✅ **Isolation**: Per-VM memory contexts + RAII cleanup
✅ **Portability**: Conditional compilation for Arduino/QEMU/STM32G4
✅ **Observability**: Telemetry observer pattern + unified error codes
✅ **Maintainability**: Single source of truth for opcodes + clean interfaces

The system is a research project, studying embedded systems with deterministic real-time execution characteristics and comprehensive diagnostic capabilities in order to develop a deeper appreciation for modern embedded practices.