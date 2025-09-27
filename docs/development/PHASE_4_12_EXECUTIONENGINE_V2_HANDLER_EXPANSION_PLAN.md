# Phase 4.12: ExecutionEngine_v2 Handler Expansion Plan

**Document Type**: Technical Implementation Plan
**Phase**: 4.12 - Complete VMOpcode Coverage
**Audience**: Senior Embedded Systems Architect Team
**Author**: cms-pm + Claude (Staff Embedded Systems Architect)
**Date**: 2025-09-27
**Classification**: TECHNICAL ARCHITECTURE
**Priority**: HIGH - Production Readiness

---

## Executive Summary

**MISSION**: Expand ExecutionEngine_v2 from current 14 handlers to complete VMOpcode coverage (58 implemented opcodes), establishing production-ready embedded hypervisor foundation for Phase 5.0 cooperative task scheduling.

**CURRENT STATUS**: ✅ ExecutionEngine_v2 Foundation Complete
- Infinite recursion eliminated via sparse jump table architecture
- GT Lite validation: 19/19 tests passing
- HALT semantics corrected
- Kill Bill VMMemoryOps elimination completed

**EXPANSION TARGET**: 44 additional handlers for 100% VMOpcode coverage

---

## Current Implementation Status

### ✅ **IMPLEMENTED (14/58 handlers - 24%)**
```cpp
// Core Operations (7/10)
OP_HALT, OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_CALL, OP_RET

// Comparison Operations (4/12)
OP_EQ, OP_NE, OP_LT, OP_GT
```

### ❌ **MISSING (44/58 handlers - 76%)**

#### **High Priority: Control Flow (3 handlers)**
```cpp
OP_JMP = 0x30,           // Unconditional jump
OP_JMP_TRUE = 0x31,      // Jump if top of stack is true
OP_JMP_FALSE = 0x32,     // Jump if top of stack is false
```

#### **High Priority: Extended Comparisons (8 handlers)**
```cpp
OP_LE = 0x24,            // Pop b, pop a, push(a <= b)
OP_GE = 0x25,            // Pop b, pop a, push(a >= b)
OP_EQ_SIGNED = 0x26,     // Signed equality
OP_NE_SIGNED = 0x27,     // Signed inequality
OP_LT_SIGNED = 0x28,     // Signed less than
OP_GT_SIGNED = 0x29,     // Signed greater than
OP_LE_SIGNED = 0x2A,     // Signed less or equal
OP_GE_SIGNED = 0x2B,     // Signed greater or equal
```

#### **Medium Priority: Logical Operations (3 handlers)**
```cpp
OP_AND = 0x40,           // Pop b, pop a, push(a && b)
OP_OR = 0x41,            // Pop b, pop a, push(a || b)
OP_NOT = 0x42,           // Pop a, push(!a)
```

#### **Medium Priority: Memory Operations (7 handlers)**
```cpp
OP_LOAD_GLOBAL = 0x50,   // Load global variable to stack
OP_STORE_GLOBAL = 0x51,  // Store stack value to global variable
OP_LOAD_LOCAL = 0x52,    // Load local variable to stack
OP_STORE_LOCAL = 0x53,   // Store stack value to local variable
OP_LOAD_ARRAY = 0x54,    // Load array element to stack
OP_STORE_ARRAY = 0x55,   // Store stack value to array element
OP_CREATE_ARRAY = 0x56,  // Allocate array in memory
```

#### **Medium Priority: Bitwise Operations (6 handlers)**
```cpp
OP_BITWISE_AND = 0x60,   // Pop b, pop a, push(a & b)
OP_BITWISE_OR = 0x61,    // Pop b, pop a, push(a | b)
OP_BITWISE_XOR = 0x62,   // Pop b, pop a, push(a ^ b)
OP_BITWISE_NOT = 0x63,   // Pop a, push(~a)
OP_SHIFT_LEFT = 0x64,    // Pop b, pop a, push(a << b)
OP_SHIFT_RIGHT = 0x65,   // Pop b, pop a, push(a >> b)
```

#### **Lower Priority: Arduino HAL Operations (11 handlers)**
```cpp
OP_DIGITAL_WRITE = 0x10, // digitalWrite(pin, value)
OP_DIGITAL_READ = 0x11,  // digitalRead(pin) -> value
OP_ANALOG_WRITE = 0x12,  // analogWrite(pin, value)
OP_ANALOG_READ = 0x13,   // analogRead(pin) -> value
OP_DELAY = 0x14,         // delay(nanoseconds)
OP_BUTTON_PRESSED = 0x15,// buttonPressed(pin) -> bool [DEPRECATED]
OP_BUTTON_RELEASED = 0x16,// buttonReleased(pin) -> bool [DEPRECATED]
OP_PIN_MODE = 0x17,      // pinMode(pin, mode)
OP_PRINTF = 0x18,        // printf(format, args...)
OP_MILLIS = 0x19,        // millis() -> timestamp
OP_MICROS = 0x1A,        // micros() -> timestamp
```

#### **Future Priority: Multimedia Operations (6 handlers)**
```cpp
OP_DISPLAY_CLEAR = 0x70, // display_clear() - Clear 128x32 OLED buffer
OP_DISPLAY_TEXT = 0x71,  // display_text(x,y,text) - Draw text at position
OP_DISPLAY_UPDATE = 0x72,// display_update() - Flush buffer to OLED hardware
OP_BUTTON_READ = 0x73,   // button_read() -> 5-bit button matrix (PC0-PC4)
OP_LED_MORSE = 0x74,     // led_morse(pattern) - Start non-blocking LED morse
```

---

## Implementation Strategy

### **Phase 4.12.1: Critical Control Flow (1-2 days)**

**Priority 1**: Control flow operations enable loops and conditionals

#### **Step 1.1: Implement Jump Operations**
```cpp
// Add to OPCODE_TABLE in execution_engine_v2.cpp
{static_cast<uint8_t>(VMOpcode::OP_JMP),       &ExecutionEngine_v2::handle_jmp_impl},
{static_cast<uint8_t>(VMOpcode::OP_JMP_TRUE),  &ExecutionEngine_v2::handle_jmp_true_impl},
{static_cast<uint8_t>(VMOpcode::OP_JMP_FALSE), &ExecutionEngine_v2::handle_jmp_false_impl},

// Handler implementations
vm_return_t ExecutionEngine_v2::handle_jmp_impl(uint16_t immediate) noexcept {
    uint32_t target_address = static_cast<uint32_t>(immediate);
    if (target_address >= program_size_) {
        return vm_return_t::error(VM_ERROR_INVALID_JUMP);
    }
    return vm_return_t::jump(target_address);
}

vm_return_t ExecutionEngine_v2::handle_jmp_true_impl(uint16_t immediate) noexcept {
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition != 0) {  // Non-zero is true
        uint32_t target_address = static_cast<uint32_t>(immediate);
        if (target_address >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(target_address);
    }

    return vm_return_t::success();  // No jump, continue normally
}

vm_return_t ExecutionEngine_v2::handle_jmp_false_impl(uint16_t immediate) noexcept {
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition == 0) {  // Zero is false
        uint32_t target_address = static_cast<uint32_t>(immediate);
        if (target_address >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(target_address);
    }

    return vm_return_t::success();  // No jump, continue normally
}
```

#### **Step 1.2: GT Lite Control Flow Tests**
```c
// tests/test_registry/lite_data/test_control_flow.c
static const uint8_t jmp_forward_bytecode[] = {
    0x01, 0x00, 0x63, 0x00,  // PUSH(99)
    0x30, 0x00, 0x03, 0x00,  // JMP(3) - Jump to instruction 3
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - Should be skipped
    0x01, 0x00, 0x17, 0x00,  // PUSH(23) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [99, 23] (42 skipped due to jump)

static const uint8_t jmp_true_taken_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH(1) - true condition
    0x31, 0x00, 0x03, 0x00,  // JMP_TRUE(3) - Should jump
    0x01, 0x00, 0x99, 0x00,  // PUSH(153) - Should be skipped
    0x01, 0x00, 0x55, 0x00,  // PUSH(85) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [85] (153 skipped, condition consumed)
```

### **Phase 4.12.2: Extended Comparisons (2-3 days)**

**Priority 2**: Complete comparison operations for comprehensive conditional logic

#### **Step 2.1: Implement Missing Unsigned Comparisons**
```cpp
vm_return_t ExecutionEngine_v2::handle_le_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Unsigned comparison
    uint32_t ua = static_cast<uint32_t>(a);
    uint32_t ub = static_cast<uint32_t>(b);
    int32_t result = (ua <= ub) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_ge_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Unsigned comparison
    uint32_t ua = static_cast<uint32_t>(a);
    uint32_t ub = static_cast<uint32_t>(b);
    int32_t result = (ua >= ub) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

#### **Step 2.2: Implement Signed Comparison Variants**
```cpp
vm_return_t ExecutionEngine_v2::handle_lt_signed_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison (a and b are already int32_t)
    int32_t result = (a < b) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

### **Phase 4.12.3: Essential Operations (3-4 days)**

**Priority 3**: Logical and bitwise operations for comprehensive programming support

#### **Step 3.1: Logical Operations**
```cpp
vm_return_t ExecutionEngine_v2::handle_and_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Logical AND: both non-zero = true (1), otherwise false (0)
    int32_t result = ((a != 0) && (b != 0)) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_not_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t a;
    if (!pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Logical NOT: zero = true (1), non-zero = false (0)
    int32_t result = (a == 0) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

#### **Step 3.2: Bitwise Operations**
```cpp
vm_return_t ExecutionEngine_v2::handle_bitwise_and_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = a & b;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_shift_left_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;  // b = shift amount, a = value to shift
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate shift amount (0-31 for 32-bit values)
    if (b < 0 || b >= 32) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    int32_t result = a << b;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

### **Phase 4.12.4: Memory Operations (3-4 days)**

**Priority 4**: Static memory integration with VMMemoryContext

#### **Step 4.1: Global Variable Operations**
```cpp
vm_return_t ExecutionEngine_v2::handle_load_global_impl(uint16_t immediate) noexcept {
    uint32_t global_index = static_cast<uint32_t>(immediate);

    // Access global variable through ComponentVM's memory context
    // Note: This requires access to memory_ (MemoryManager reference)
    int32_t value;
    if (!memory_->get_global_variable(global_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_global_impl(uint16_t immediate) noexcept {
    uint32_t global_index = static_cast<uint32_t>(immediate);

    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (!memory_->set_global_variable(global_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    return vm_return_t::success();
}
```

### **Phase 4.12.5: Arduino HAL Integration (4-5 days)**

**Priority 5**: Hardware abstraction layer operations

#### **Step 5.1: GPIO Operations**
```cpp
vm_return_t ExecutionEngine_v2::handle_digital_write_impl(uint16_t immediate) noexcept {
    (void)immediate;  // Pin and value come from stack

    int32_t value, pin;
    if (!pop_protected(value) || !pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range and value
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_PIN);
    }

    if (value < 0 || value > 1) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    // Use IOController interface
    if (!io_->digital_write(static_cast<uint8_t>(pin), value != 0)) {
        return vm_return_t::error(VM_ERROR_IO_OPERATION_FAILED);
    }

    return vm_return_t::success();
}
```

---

## Testing Strategy

### **GT Lite Test Suite Expansion**

#### **Phase 4.12.1 Tests: Control Flow**
```bash
# New test suites to create
tests/test_registry/lite_src/test_lite_control_flow.c
tests/test_registry/lite_data/test_control_flow.c
```

#### **Phase 4.12.2-5 Tests: Comprehensive Coverage**
```bash
tests/test_registry/lite_src/test_lite_logical.c
tests/test_registry/lite_src/test_lite_bitwise.c
tests/test_registry/lite_src/test_lite_memory.c
tests/test_registry/lite_src/test_lite_arduino_hal.c
```

#### **Performance Validation**
```bash
# Verify no performance regression with expanded handlers
time USE_EXECUTION_ENGINE_V2=1 ./tools/run_test test_lite_stack      # Baseline
time USE_EXECUTION_ENGINE_V2=1 ./tools/run_test test_lite_control_flow # New
time USE_EXECUTION_ENGINE_V2=1 ./tools/run_test test_lite_comprehensive # Full coverage
```

### **Binary Search Table Performance**

Current sparse table lookup: **O(log 14) ≈ 3.8 comparisons**
Expanded sparse table lookup: **O(log 58) ≈ 5.9 comparisons**
Memory usage: **~460 bytes vs 1KB traditional table (55% reduction)**

---

## Migration Strategy

### **Incremental Implementation Pattern**

1. **Add 3 handlers per implementation session**
2. **Update OPCODE_TABLE in sorted order** (critical for binary search)
3. **Create corresponding GT Lite tests immediately**
4. **Validate no regressions in existing 19 passing tests**
5. **Document handler behavior and edge cases**

### **Error Handling Consistency**

All new handlers follow ExecutionEngine_v2 patterns:
- **Stack protection**: `pop_protected()` / `push_protected()`
- **Unified error system**: `vm_return_t::error(VM_ERROR_*)`
- **Validation**: Parameter range checking before operations
- **Clean state**: No partial operations on error conditions

### **Integration Points**

#### **MemoryManager Integration**
Memory operations require access to static VMMemoryContext:
```cpp
// ExecutionEngine_v2 constructor updates
ExecutionEngine_v2(MemoryManager& memory, IOController& io)
    : memory_(&memory), io_(&io)
{
    // Direct references for handler access
}
```

#### **IOController Integration**
Arduino HAL operations require hardware abstraction:
```cpp
// GPIO validation and hardware safety
bool validate_pin_configuration(uint8_t pin, bool is_output) const;
bool perform_safe_io_operation(uint8_t pin, IOOperation op, int32_t value);
```

---

## Success Metrics

### **Coverage Goals**
- **Phase 4.12.1**: 17/58 handlers (30%) - Control flow operational
- **Phase 4.12.2**: 25/58 handlers (43%) - Extended comparisons complete
- **Phase 4.12.3**: 34/58 handlers (59%) - Essential operations complete
- **Phase 4.12.4**: 41/58 handlers (71%) - Memory operations integrated
- **Phase 4.12.5**: 52/58 handlers (90%) - Arduino HAL operational
- **Phase 4.12.6**: 58/58 handlers (100%) - Complete VMOpcode coverage

### **Quality Requirements**
- **GT Lite validation**: All test suites maintain 100% pass rate
- **Performance**: No degradation beyond O(log n) lookup increase
- **Memory efficiency**: Sparse table remains <500 bytes total
- **Error handling**: Comprehensive validation with unified error system
- **Documentation**: Each handler documented with behavior specification

### **Production Readiness Validation**
- **STM32G474 hardware testing**: Physical device validation
- **Memory constraints**: 128KB flash, 32KB RAM compliance
- **Real-time performance**: Deterministic execution timing
- **Safety systems**: Stack canary validation and bounds checking

---

## Risk Assessment and Mitigation

### **High Risk: Memory Operations Complexity**
**Mitigation**: Implement memory operations last, after control flow and arithmetic proven stable

### **Medium Risk: Arduino HAL Hardware Dependencies**
**Mitigation**: Use IOController abstraction layer, platform-specific validation in test harness

### **Low Risk: Binary Search Table Corruption**
**Mitigation**: Sorted table validation in debug builds, compile-time assertions for order

### **Low Risk: Performance Regression**
**Mitigation**: Continuous GT Lite performance benchmarking at each phase

---

## Future Integration Points

### **Phase 5.0: Cooperative Task Scheduling**
Complete VMOpcode coverage enables:
- **Multi-program switching** via CALL/RET and memory isolation
- **Static memory allocation** through array and global operations
- **Hardware coordination** via Arduino HAL operations
- **Deterministic execution** through comprehensive instruction timing

### **Phase 5.1: Preemptive RTOS Architecture**
Handler expansion provides foundation for:
- **Context switching** using complete register and memory state
- **Priority-based scheduling** through performance instrumentation
- **Hardware timer integration** via timing operations
- **Inter-task communication** through memory and I/O operations

---

**IMPLEMENTATION READY**: This plan provides systematic progression from current 24% handler coverage to complete 100% VMOpcode implementation, establishing production-ready embedded hypervisor foundation for advanced scheduling architectures.

**NEXT ACTION**: Begin Phase 4.12.1 - Implement critical control flow operations (JMP, JMP_TRUE, JMP_FALSE) with corresponding GT Lite validation suite.