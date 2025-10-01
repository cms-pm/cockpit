# CockpitVM API Reference

**Version**: Phase 4.14
**Date**: September 2025
**Target**: STM32G474 WeAct CoreBoard (ARM Cortex-M4)
**Status**: Research-grade embedded hypervisor

---

## Table of Contents

- [Overview](#overview)
- [System Requirements](#system-requirements)
- [Core API](#core-api)
- [VM State Management](#vm-state-management)
- [Error Handling](#error-handling)
- [Performance Monitoring](#performance-monitoring)
- [Observer Pattern](#observer-pattern)
- [Data Structures](#data-structures)
- [Code Examples](#code-examples)
- [Best Practices](#best-practices)

---

## Overview

CockpitVM is an embedded hypervisor implementing a stack-based virtual machine for ARM Cortex-M4 microcontrollers. The system executes guest bytecode programs with hardware isolation and Arduino HAL compatibility.

**Key Features**:
- **Stack-Based VM**: 1024-element stack with overflow/underflow protection
- **Arduino HAL**: pinMode(), digitalWrite(), delay(), millis(), printf()
- **Memory Safety**: Stack canaries, bounds checking, corruption detection
- **Hardware Abstraction**: Platform-independent bytecode execution
- **Debug Support**: Observer pattern for execution telemetry

**Architecture**:
```
Guest Bytecode → ComponentVM → ExecutionEngine → MemoryManager + IOController → STM32 HAL
```

---

## System Requirements

**Hardware Platform**: STM32G474 WeAct CoreBoard
- Flash: 128KB (hypervisor + guest programs)
- RAM: 32KB (VM stack + heap + globals)
- Clock: 168MHz ARM Cortex-M4

**Memory Allocation**:
```cpp
#define VM_STACK_SIZE      1024    // 1024 int32_t elements (4KB)
#define VM_HEAP_SIZE       4096    // 4KB heap space
#define VM_GLOBALS_SIZE    256     // 256 bytes global variables
```

**Build Configuration**:
```cpp
// Enable ExecutionEngine_v2 (Phase 4.13+)
#define USE_EXECUTION_ENGINE_V2

// Enable GT Lite testing support
#define ENABLE_GT_LITE_TESTING

// Debug builds only
#ifdef DEBUG
#define ENABLE_VM_TRACING
#endif
```

---

## Core API

### ComponentVM Class

Main VM interface for program execution and management.

#### Constructor

```cpp
ComponentVM() noexcept;
```

Creates ComponentVM instance with default memory context.

**Example**:
```cpp
ComponentVM vm;
```

#### Alternative Constructor (Phase 4.14.1+)

```cpp
explicit ComponentVM(VMMemoryContext_t context) noexcept;
```

Creates ComponentVM with custom memory context (advanced usage).

### Program Execution

#### execute_program()

```cpp
bool execute_program(const VM::Instruction* program, size_t program_size) noexcept;
```

Load and execute complete bytecode program.

**Parameters**:
- `program` - Array of VM instructions
- `program_size` - Number of instructions

**Returns**: `true` on success, `false` on error

**Example**:
```cpp
ComponentVM vm;

VM::Instruction blink_program[] = {
    {0x17, 0, 13},    // pinMode(13, OUTPUT)
    {0x10, 0, 1},     // Push HIGH
    {0x10, 0, 13},    // digitalWrite(13, HIGH)
    {0x01, 0, 500},   // Push 500
    {0x14, 0, 0},     // delay(500)
    {0x00, 0, 0}      // HALT
};

if (vm.execute_program(blink_program, 6)) {
    printf("Program completed successfully\n");
} else {
    printf("Execution failed: %s\n",
           vm.get_error_string(vm.get_last_error()));
}
```

#### execute_single_step()

```cpp
bool execute_single_step() noexcept;
```

Execute exactly one instruction for debugging.

**Returns**: `true` on success, `false` on error or halt

**Example**:
```cpp
vm.load_program(program, program_size);

while (!vm.is_halted()) {
    printf("PC: %zu\n", vm.get_execution_engine().get_pc());

    if (!vm.execute_single_step()) {
        printf("Step failed\n");
        break;
    }
}
```

### Program Management

#### load_program()

```cpp
bool load_program(const VM::Instruction* program, size_t program_size) noexcept;
```

Load bytecode program without executing (for step-by-step debugging).

#### load_program_with_strings()

```cpp
bool load_program_with_strings(const VM::Instruction* program,
                              size_t program_size,
                              const char* const* string_literals,
                              size_t string_count) noexcept;
```

Load program with printf format strings.

**Example**:
```cpp
VM::Instruction printf_program[] = {
    {0x01, 0, 42},    // Push value
    {0x18, 0, 0},     // Printf (string_id=0)
    {0x00, 0, 0}      // HALT
};

const char* strings[] = {
    "Value: %d\n"
};

vm.load_program_with_strings(printf_program, 3, strings, 1);
vm.execute_program(printf_program, 3);
```

#### reset_vm()

```cpp
void reset_vm() noexcept;
```

Reset VM to initial state while preserving loaded program.

---

## VM State Management

### State Inspection

#### is_running()

```cpp
bool is_running() const noexcept;
```

Check if VM is currently executing instructions.

#### is_halted()

```cpp
bool is_halted() const noexcept;
```

Check if VM has halted (HALT instruction or completion).

#### get_instruction_count()

```cpp
size_t get_instruction_count() const noexcept;
```

Get total instructions executed since last reset.

**Example**:
```cpp
vm.execute_program(program, size);
printf("Executed %zu instructions\n", vm.get_instruction_count());
```

### Component Access

#### get_execution_engine()

```cpp
ExecutionEngine_v2& get_execution_engine() noexcept;
const ExecutionEngine_v2& get_execution_engine() const noexcept;
```

Access execution engine for PC/SP inspection.

**Methods**:
```cpp
size_t get_pc() const noexcept;      // Program counter
size_t get_sp() const noexcept;      // Stack pointer
bool is_halted() const noexcept;     // Halt state
```

#### get_memory_manager()

```cpp
MemoryManager& get_memory_manager() noexcept;
const MemoryManager& get_memory_manager() const noexcept;
```

Access memory manager for global/local variables.

#### get_io_controller()

```cpp
IOController& get_io_controller() noexcept;
const IOController& get_io_controller() const noexcept;
```

Access I/O controller for hardware operations.

---

## Error Handling

### Error Codes

```cpp
enum class vm_error_t : uint8_t {
    VM_ERROR_NONE = 0,                // Success
    VM_ERROR_STACK_OVERFLOW = 1,      // Stack exceeded limit
    VM_ERROR_STACK_UNDERFLOW = 2,     // Pop from empty stack
    VM_ERROR_INVALID_OPCODE = 3,      // Unknown instruction
    VM_ERROR_DIVISION_BY_ZERO = 4,    // Arithmetic error
    VM_ERROR_INVALID_JUMP = 5,        // Jump to invalid address
    VM_ERROR_HARDWARE_FAULT = 6,      // GPIO/hardware error
    VM_ERROR_MEMORY_ACCESS = 7,       // Memory bounds violation
    VM_ERROR_PRINTF_ERROR = 8         // Printf formatting failed
};
```

### Error Handling API

#### get_last_error()

```cpp
vm_error_t get_last_error() const noexcept;
```

Retrieve most recent error code.

#### get_error_string()

```cpp
const char* get_error_string(vm_error_t error) const noexcept;
```

Convert error code to human-readable string.

**Example**:
```cpp
if (!vm.execute_program(program, size)) {
    vm_error_t error = vm.get_last_error();
    printf("Error %d: %s\n",
           static_cast<int>(error),
           vm.get_error_string(error));

    switch (error) {
        case vm_error_t::VM_ERROR_STACK_OVERFLOW:
            printf("Reduce recursion depth\n");
            break;
        case vm_error_t::VM_ERROR_DIVISION_BY_ZERO:
            printf("Add zero-check before division\n");
            break;
        default:
            break;
    }
}
```

---

## Performance Monitoring

### Performance Metrics

```cpp
struct PerformanceMetrics {
    uint32_t execution_time_ms;      // Total execution time
    size_t instructions_executed;    // Instructions processed
    size_t memory_operations;        // Stack/memory accesses
    size_t io_operations;            // GPIO/Arduino operations
};
```

### Performance API

#### get_performance_metrics()

```cpp
PerformanceMetrics get_performance_metrics() const noexcept;
```

Retrieve current performance metrics.

#### reset_performance_metrics()

```cpp
void reset_performance_metrics() noexcept;
```

Reset performance counters.

**Example**:
```cpp
vm.reset_performance_metrics();
vm.execute_program(program, size);

auto metrics = vm.get_performance_metrics();
printf("Time: %u ms\n", metrics.execution_time_ms);
printf("Instructions: %zu\n", metrics.instructions_executed);
printf("IPS: %zu\n",
       metrics.instructions_executed * 1000 / metrics.execution_time_ms);
```

---

## Observer Pattern

### ITelemetryObserver Interface

```cpp
class ITelemetryObserver {
public:
    virtual ~ITelemetryObserver() = default;

    virtual void on_instruction_executed(uint32_t pc, uint8_t opcode,
                                        uint32_t operand) = 0;
    virtual void on_execution_complete(uint32_t total_instructions,
                                      uint32_t execution_time_ms) = 0;
    virtual void on_execution_error(uint32_t pc, uint8_t opcode,
                                   uint32_t operand, vm_error_t error) = 0;
    virtual void on_vm_reset() = 0;
};
```

### Observer Management

#### add_observer()

```cpp
void add_observer(ITelemetryObserver* observer) noexcept;
```

Register telemetry observer for execution monitoring.

#### remove_observer()

```cpp
void remove_observer(ITelemetryObserver* observer) noexcept;
```

Unregister telemetry observer.

**Example**:
```cpp
class DebugObserver : public ITelemetryObserver {
public:
    void on_instruction_executed(uint32_t pc, uint8_t opcode,
                                uint32_t operand) override {
        printf("PC=%u opcode=0x%02x operand=%u\n", pc, opcode, operand);
    }

    void on_execution_complete(uint32_t total, uint32_t time_ms) override {
        printf("Complete: %u instructions in %u ms\n", total, time_ms);
    }

    void on_execution_error(uint32_t pc, uint8_t opcode,
                           uint32_t operand, vm_error_t error) override {
        printf("Error at PC=%u: %d\n", pc, static_cast<int>(error));
    }

    void on_vm_reset() override {
        printf("VM reset\n");
    }
};

DebugObserver observer;
vm.add_observer(&observer);
vm.execute_program(program, size);
vm.remove_observer(&observer);
```

---

## Data Structures

### VM::Instruction

32-bit packed instruction format for ARM optimization.

```cpp
namespace VM {
    struct Instruction {
        uint8_t  opcode;     // 256 base operations (0x00-0xFF)
        uint8_t  flags;      // 8 modifier bits
        uint16_t immediate;  // 0-65535 operand value
    } __attribute__((packed));
}
```

**Instruction Examples**:
```cpp
// Stack operations
{0x01, 0, 42}       // PUSH 42

// Arithmetic
{0x03, 0, 0}        // ADD (pop b, pop a, push a+b)
{0x04, 0, 0}        // SUB
{0x05, 0, 0}        // MUL
{0x06, 0, 0}        // DIV

// Control flow
{0x30, 0, 10}       // JMP to instruction 10
{0x31, 0, 5}        // JMP_TRUE to instruction 5
{0x32, 0, 2}        // JMP_FALSE to instruction 2

// Arduino HAL
{0x17, 0, 13}       // pinMode(13, OUTPUT)
{0x10, 0, 1}        // digitalWrite(pin, HIGH)
{0x11, 0, 0}        // digitalRead(pin)
{0x14, 0, 500}      // delay(500 ms)
{0x19, 0, 0}        // millis()
{0x18, 0, 0}        // printf(string_id=0)

// Halt
{0x00, 0, 0}        // HALT
```

### VMMemoryContext_t

Memory context for advanced VM configuration (Phase 4.14.1+).

```cpp
struct VMMemoryContext_t {
    uint8_t* stack_memory;
    size_t stack_size;
    uint8_t* heap_memory;
    size_t heap_size;
    uint8_t* global_memory;
    size_t global_size;
};
```

**Factory Pattern**:
```cpp
VMMemoryContext_t create_standard_context();
VMMemoryContext_t create_custom_context(size_t stack_kb,
                                       size_t heap_kb,
                                       size_t globals_bytes);
```

---

## Code Examples

### Basic LED Blink

```cpp
#include "component_vm.h"

int main() {
    ComponentVM vm;

    VM::Instruction blink[] = {
        {0x17, 0, 13},    // pinMode(13, OUTPUT)
        {0x01, 0, 1},     // PUSH HIGH
        {0x10, 0, 13},    // digitalWrite(13, HIGH)
        {0x01, 0, 500},   // PUSH 500
        {0x14, 0, 0},     // delay(500)
        {0x01, 0, 0},     // PUSH LOW
        {0x10, 0, 13},    // digitalWrite(13, LOW)
        {0x01, 0, 500},   // PUSH 500
        {0x14, 0, 0},     // delay(500)
        {0x00, 0, 0}      // HALT
    };

    if (vm.execute_program(blink, 10)) {
        printf("Blink complete\n");
    }

    return 0;
}
```

### Arithmetic with Error Handling

```cpp
ComponentVM vm;

// Calculate: (10 + 5) * 2
VM::Instruction math[] = {
    {0x01, 0, 10},    // PUSH 10
    {0x01, 0, 5},     // PUSH 5
    {0x03, 0, 0},     // ADD (result: 15)
    {0x01, 0, 2},     // PUSH 2
    {0x05, 0, 0},     // MUL (result: 30)
    {0x00, 0, 0}      // HALT
};

if (!vm.execute_program(math, 6)) {
    vm_error_t error = vm.get_last_error();
    printf("Math failed: %s\n", vm.get_error_string(error));
    return 1;
}

// Result is on stack top
int32_t result;
if (vm.get_execution_engine().peek(result)) {
    printf("Result: %d\n", result);  // 30
}
```

### Debug Tracing with Observer

```cpp
class TraceObserver : public ITelemetryObserver {
    size_t step_ = 0;

public:
    void on_instruction_executed(uint32_t pc, uint8_t opcode,
                                uint32_t operand) override {
        printf("[%zu] PC=%u OP=0x%02x IMM=%u\n",
               step_++, pc, opcode, operand);
    }

    void on_execution_complete(uint32_t total, uint32_t time_ms) override {
        printf("Done: %u instructions, %u ms\n", total, time_ms);
    }

    void on_execution_error(uint32_t pc, uint8_t opcode,
                           uint32_t operand, vm_error_t error) override {
        printf("ERROR at PC=%u: %d\n", pc, static_cast<int>(error));
    }

    void on_vm_reset() override { step_ = 0; }
};

ComponentVM vm;
TraceObserver trace;

vm.add_observer(&trace);
vm.execute_program(program, size);
vm.remove_observer(&trace);
```

### Performance Benchmarking

```cpp
void benchmark_loop(size_t iterations) {
    ComponentVM vm;

    // Loop N iterations: for (i=N; i>0; i--)
    VM::Instruction loop[] = {
        {0x01, 0, static_cast<uint16_t>(iterations)},  // PUSH N
        {0x01, 0, 1},                                  // PUSH 1
        {0x04, 0, 0},                                  // SUB
        {0x32, 0, 1},                                  // JMP_FALSE to 1
        {0x00, 0, 0}                                   // HALT
    };

    vm.reset_performance_metrics();

    if (vm.execute_program(loop, 5)) {
        auto metrics = vm.get_performance_metrics();
        printf("Iterations: %zu\n", iterations);
        printf("Time: %u ms\n", metrics.execution_time_ms);
        printf("Instructions: %zu\n", metrics.instructions_executed);
        printf("IPS: %zu\n",
               metrics.instructions_executed * 1000 /
               metrics.execution_time_ms);
    }
}

benchmark_loop(1000);
benchmark_loop(10000);
```

---

## Best Practices

### Memory Safety

```cpp
// ✓ GOOD: Check execution results
if (!vm.execute_program(program, size)) {
    handle_error(vm.get_last_error());
}

// ✗ BAD: Ignoring errors
vm.execute_program(program, size);  // No error check
```

### Resource Management

```cpp
// ✓ GOOD: Reset between runs
ComponentVM vm;
for (int i = 0; i < 5; i++) {
    vm.execute_program(test, size);
    vm.reset_vm();  // Clean state
}

// ✗ BAD: Reusing without reset
for (int i = 0; i < 5; i++) {
    vm.execute_program(test, size);  // Undefined state
}
```

### Performance Optimization

```cpp
// ✓ GOOD: Profile before optimizing
vm.reset_performance_metrics();
vm.execute_program(program, size);
auto metrics = vm.get_performance_metrics();

if (metrics.execution_time_ms > THRESHOLD) {
    optimize_critical_path();
}

// ✗ BAD: Premature optimization
optimize_everything();  // Without measurement
```

### Hardware Integration

```cpp
// ✓ GOOD: Validate GPIO operations
VM::Instruction gpio_test[] = {
    {0x17, 0, 13},    // pinMode(13, OUTPUT)
    {0x10, 0, 1},     // digitalWrite(13, HIGH)
    {0x00, 0, 0}      // HALT
};

if (!vm.execute_program(gpio_test, 3)) {
    if (vm.get_last_error() == vm_error_t::VM_ERROR_HARDWARE_FAULT) {
        check_hardware_connections();
    }
}
```

### Debug Builds

```cpp
#ifdef DEBUG
    // Enable detailed tracing for debug builds
    class DetailedTrace : public ITelemetryObserver { /* ... */ };
    DetailedTrace trace;
    vm.add_observer(&trace);
#endif

vm.execute_program(program, size);

#ifdef DEBUG
    vm.remove_observer(&trace);
#endif
```

---

## Hardware-Specific Notes

### STM32G474 Performance

**Typical Metrics** (168MHz ARM Cortex-M4):
- Instructions/second: ~100,000 IPS
- Function call overhead: 7 CPU cycles
- GPIO operation: 15-20 CPU cycles
- Stack push/pop: 2-3 CPU cycles

### Memory Layout

```
Flash (128KB):
├─ Bootloader (16KB)
├─ Hypervisor (80KB)
└─ Guest Programs (32KB)

RAM (32KB):
├─ VM Stack (4KB)
├─ VM Heap (4KB)
├─ VM Globals (256B)
└─ System (23KB)
```

---

*This API reference covers the complete ComponentVM interface for embedded hypervisor development on STM32G474 hardware.*
