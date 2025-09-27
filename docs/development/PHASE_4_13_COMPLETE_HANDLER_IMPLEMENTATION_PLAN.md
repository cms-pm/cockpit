# Phase 4.13: Complete Handler Implementation & Test Coverage Plan

**Document Type**: Technical Implementation Plan
**Phase**: 4.13 - Complete VMOpcode Handler Coverage
**Audience**: Senior Embedded Systems Architect Team
**Author**: cms-pm + Claude (Staff Embedded Systems Architect)
**Date**: 2025-09-27
**Classification**: TECHNICAL ARCHITECTURE
**Priority**: CRITICAL - Production Completion

---

## Executive Summary

**MISSION**: Implement remaining 44 handlers for complete VMOpcode coverage (58 total) with comprehensive GT Lite test validation, establishing production-ready embedded hypervisor.

**CURRENT STATUS**: 14/58 handlers implemented (24%)
- ✅ Core Operations: HALT, PUSH, POP, ADD, SUB, MUL, DIV, MOD, CALL, RET
- ✅ Basic Comparisons: EQ, NE, LT, GT
- ✅ GT Lite Validation: 19/19 tests passing

**TARGET**: 44 additional handlers with comprehensive test coverage
- Control Flow Operations (3 handlers)
- Extended Comparisons (8 handlers)
- Logical Operations (3 handlers)
- Memory Operations (7 handlers)
- Bitwise Operations (6 handlers)
- Arduino HAL Operations (11 handlers)
- Multimedia Operations (6 handlers)

---

## Complete Handler Analysis

### **✅ IMPLEMENTED (14/58 handlers)**
```cpp
// Core Operations (10/10 in scope)
OP_HALT = 0x00,    ✅    OP_MOD = 0x07,      ✅
OP_PUSH = 0x01,    ✅    OP_CALL = 0x08,     ✅
OP_POP = 0x02,     ✅    OP_RET = 0x09,      ✅
OP_ADD = 0x03,     ✅
OP_SUB = 0x04,     ✅    // Comparison Operations (4/12)
OP_MUL = 0x05,     ✅    OP_EQ = 0x20,       ✅
OP_DIV = 0x06,     ✅    OP_NE = 0x21,       ✅
                         OP_LT = 0x22,       ✅
                         OP_GT = 0x23,       ✅
```

### **❌ MISSING (44/58 handlers) - SYSTEMATIC IMPLEMENTATION PLAN**

---

## Phase 4.13.1: Control Flow Operations (3 handlers) [CRITICAL PRIORITY]

### **Implementation Target: 17/58 handlers (30%)**

**Strategic Importance**: Control flow enables loops, conditionals, and program structure

#### **Handlers to Implement**
```cpp
OP_JMP = 0x30,           // Unconditional jump
OP_JMP_TRUE = 0x31,      // Jump if top of stack is true
OP_JMP_FALSE = 0x32,     // Jump if top of stack is false
```

#### **ExecutionEngine_v2 Implementation**
```cpp
// Add to OPCODE_TABLE in execution_engine_v2.cpp (maintain sorted order)
{static_cast<uint8_t>(VMOpcode::OP_JMP),       &ExecutionEngine_v2::handle_jmp_impl},
{static_cast<uint8_t>(VMOpcode::OP_JMP_TRUE),  &ExecutionEngine_v2::handle_jmp_true_impl},
{static_cast<uint8_t>(VMOpcode::OP_JMP_FALSE), &ExecutionEngine_v2::handle_jmp_false_impl},

// Handler implementations
vm_return_t ExecutionEngine_v2::handle_jmp_impl(uint16_t immediate) noexcept {
    uint32_t target_address = static_cast<uint32_t>(immediate);

    // Validate jump target
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

    if (condition != 0) {  // Non-zero is true in C semantics
        uint32_t target_address = static_cast<uint32_t>(immediate);
        if (target_address >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(target_address);
    }

    return vm_return_t::success();  // Continue to next instruction
}

vm_return_t ExecutionEngine_v2::handle_jmp_false_impl(uint16_t immediate) noexcept {
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition == 0) {  // Zero is false in C semantics
        uint32_t target_address = static_cast<uint32_t>(immediate);
        if (target_address >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(target_address);
    }

    return vm_return_t::success();  // Continue to next instruction
}
```

#### **GT Lite Test Suite: Control Flow**
```c
// tests/test_registry/lite_data/test_control_flow.c

// Test 1: Unconditional jump forward
static const uint8_t jmp_forward_bytecode[] = {
    0x01, 0x00, 0x63, 0x00,  // PUSH(99)
    0x30, 0x00, 0x04, 0x00,  // JMP(4) - Jump to instruction 4
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - Should be skipped
    0x01, 0x00, 0x17, 0x00,  // PUSH(23) - Jump target (instruction 4)
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [99, 23] (42 skipped due to jump)

// Test 2: Conditional jump taken (true condition)
static const uint8_t jmp_true_taken_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH(1) - true condition
    0x31, 0x00, 0x04, 0x00,  // JMP_TRUE(4) - Should jump
    0x01, 0x00, 0x99, 0x00,  // PUSH(153) - Should be skipped
    0x01, 0x00, 0x55, 0x00,  // PUSH(85) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [85] (condition consumed, 153 skipped)

// Test 3: Conditional jump not taken (false condition)
static const uint8_t jmp_true_not_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH(0) - false condition
    0x31, 0x00, 0x04, 0x00,  // JMP_TRUE(4) - Should NOT jump
    0x01, 0x00, 0x7B, 0x00,  // PUSH(123) - Should execute
    0x01, 0x00, 0x55, 0x00,  // PUSH(85) - Should execute
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [123, 85] (no jump, all instructions execute)

// Test 4: JMP_FALSE with false condition (jump taken)
static const uint8_t jmp_false_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH(0) - false condition
    0x32, 0x00, 0x04, 0x00,  // JMP_FALSE(4) - Should jump
    0x01, 0x00, 0x88, 0x00,  // PUSH(136) - Should be skipped
    0x01, 0x00, 0x77, 0x00,  // PUSH(119) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [119] (condition consumed, 136 skipped)

// Test 5: Invalid jump address (error condition)
static const uint8_t jmp_invalid_address_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH(1)
    0x30, 0x00, 0xFF, 0x00,  // JMP(255) - Invalid address (beyond program)
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};
// Expected error: VM_ERROR_INVALID_JUMP

// Test 6: Stack underflow on conditional jump
static const uint8_t jmp_stack_underflow_bytecode[] = {
    0x31, 0x00, 0x02, 0x00,  // JMP_TRUE(2) - No condition on stack
    0x01, 0x00, 0x42, 0x00,  // PUSH(66) - Never reached
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected error: VM_ERROR_STACK_UNDERFLOW

const gt_lite_test_suite_t control_flow_test_suite = {
    .suite_name = "control_flow_operations",
    .test_count = 6,
    .tests = (const gt_lite_test_t[]){
        {
            .test_name = "jmp_forward",
            .bytecode = jmp_forward_bytecode,
            .bytecode_size = sizeof(jmp_forward_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {99, 23},
            .expected_stack_size = 2
        },
        {
            .test_name = "jmp_true_taken",
            .bytecode = jmp_true_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {85},
            .expected_stack_size = 1
        },
        {
            .test_name = "jmp_true_not_taken",
            .bytecode = jmp_true_not_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_not_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {123, 85},
            .expected_stack_size = 2
        },
        {
            .test_name = "jmp_false_taken",
            .bytecode = jmp_false_taken_bytecode,
            .bytecode_size = sizeof(jmp_false_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {119},
            .expected_stack_size = 1
        },
        {
            .test_name = "jmp_invalid_address",
            .bytecode = jmp_invalid_address_bytecode,
            .bytecode_size = sizeof(jmp_invalid_address_bytecode),
            .expected_error = GT_LITE_VM_ERROR_INVALID_JUMP,
            .expected_stack = {},
            .expected_stack_size = 0
        },
        {
            .test_name = "jmp_stack_underflow",
            .bytecode = jmp_stack_underflow_bytecode,
            .bytecode_size = sizeof(jmp_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0
        }
    }
};
```

---

## Phase 4.13.2: Extended Comparisons (8 handlers) [HIGH PRIORITY]

### **Implementation Target: 25/58 handlers (43%)**

#### **Handlers to Implement**
```cpp
// Unsigned comparisons
OP_LE = 0x24,            // Pop b, pop a, push(a <= b)
OP_GE = 0x25,            // Pop b, pop a, push(a >= b)

// Signed comparisons
OP_EQ_SIGNED = 0x26,     // Signed equality
OP_NE_SIGNED = 0x27,     // Signed inequality
OP_LT_SIGNED = 0x28,     // Signed less than
OP_GT_SIGNED = 0x29,     // Signed greater than
OP_LE_SIGNED = 0x2A,     // Signed less or equal
OP_GE_SIGNED = 0x2B,     // Signed greater or equal
```

#### **Implementation Pattern**
```cpp
vm_return_t ExecutionEngine_v2::handle_le_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Unsigned comparison (cast to uint32_t)
    uint32_t ua = static_cast<uint32_t>(a);
    uint32_t ub = static_cast<uint32_t>(b);
    int32_t result = (ua <= ub) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_lt_signed_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison (a and b already int32_t)
    int32_t result = (a < b) ? 1 : 0;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

---

## Phase 4.13.3: Logical Operations (3 handlers) [HIGH PRIORITY]

### **Implementation Target: 28/58 handlers (48%)**

#### **Handlers to Implement**
```cpp
OP_AND = 0x40,           // Pop b, pop a, push(a && b)
OP_OR = 0x41,            // Pop b, pop a, push(a || b)
OP_NOT = 0x42,           // Pop a, push(!a)
```

#### **Implementation with C Logical Semantics**
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

vm_return_t ExecutionEngine_v2::handle_or_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Logical OR: either non-zero = true (1), both zero = false (0)
    int32_t result = ((a != 0) || (b != 0)) ? 1 : 0;

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

---

## Phase 4.13.4: Memory Operations (7 handlers) [MEDIUM PRIORITY]

### **Implementation Target: 35/58 handlers (60%)**

#### **Handlers to Implement**
```cpp
OP_LOAD_GLOBAL = 0x50,   // Load global variable to stack
OP_STORE_GLOBAL = 0x51,  // Store stack value to global variable
OP_LOAD_LOCAL = 0x52,    // Load local variable to stack
OP_STORE_LOCAL = 0x53,   // Store stack value to local variable
OP_LOAD_ARRAY = 0x54,    // Load array element to stack
OP_STORE_ARRAY = 0x55,   // Store stack value to array element
OP_CREATE_ARRAY = 0x56,  // Allocate array in memory
```

#### **MemoryManager Integration**
```cpp
vm_return_t ExecutionEngine_v2::handle_load_global_impl(uint16_t immediate) noexcept {
    uint8_t global_index = static_cast<uint8_t>(immediate);

    int32_t value;
    if (!memory_->load_global(global_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_global_impl(uint16_t immediate) noexcept {
    uint8_t global_index = static_cast<uint8_t>(immediate);

    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (!memory_->store_global(global_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_create_array_impl(uint16_t immediate) noexcept {
    // Array size comes from stack, array_id from immediate
    uint8_t array_id = static_cast<uint8_t>(immediate);

    int32_t size_value;
    if (!pop_protected(size_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (size_value <= 0 || size_value > static_cast<int32_t>(MemoryManager::MAX_ARRAY_SIZE)) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    size_t array_size = static_cast<size_t>(size_value);
    if (!memory_->create_array(array_id, array_size)) {
        return vm_return_t::error(VM_ERROR_MEMORY_ALLOCATION_FAILED);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_load_array_impl(uint16_t immediate) noexcept {
    uint8_t array_id = static_cast<uint8_t>(immediate);

    // Array index comes from stack
    int32_t index_value;
    if (!pop_protected(index_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (index_value < 0) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    uint16_t array_index = static_cast<uint16_t>(index_value);
    int32_t value;
    if (!memory_->load_array_element(array_id, array_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_array_impl(uint16_t immediate) noexcept {
    uint8_t array_id = static_cast<uint8_t>(immediate);

    // Pop value and index from stack (value first, then index)
    int32_t value, index_value;
    if (!pop_protected(value) || !pop_protected(index_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (index_value < 0) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    uint16_t array_index = static_cast<uint16_t>(index_value);
    if (!memory_->store_array_element(array_id, array_index, value)) {
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }

    return vm_return_t::success();
}
```

#### **Memory Operations Test Suite**
```c
// tests/test_registry/lite_data/test_memory.c

// Test 1: Global variable store and load
static const uint8_t global_store_load_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42)
    0x51, 0x00, 0x05, 0x00,  // STORE_GLOBAL(5) - Store to global[5]
    0x50, 0x00, 0x05, 0x00,  // LOAD_GLOBAL(5) - Load from global[5]
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [42] (stored and retrieved from global[5])

// Test 2: Array creation and element operations
static const uint8_t array_operations_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH(10) - Array size
    0x56, 0x00, 0x02, 0x00,  // CREATE_ARRAY(2) - Create array_id=2, size=10
    0x01, 0x00, 0x03, 0x00,  // PUSH(3) - Array index
    0x01, 0x00, 0x7B, 0x00,  // PUSH(123) - Value to store
    0x55, 0x00, 0x02, 0x00,  // STORE_ARRAY(2) - Store to array[2][3]
    0x01, 0x00, 0x03, 0x00,  // PUSH(3) - Array index for load
    0x54, 0x00, 0x02, 0x00,  // LOAD_ARRAY(2) - Load from array[2][3]
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack = [123] (stored and retrieved from array[2][3])

// Test 3: Invalid global index (error condition)
static const uint8_t invalid_global_bytecode[] = {
    0x01, 0x00, 0x42, 0x00,  // PUSH(66)
    0x51, 0x00, 0xFF, 0x00,  // STORE_GLOBAL(255) - Invalid index
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};
// Expected error: VM_ERROR_INVALID_MEMORY_ACCESS
```

---

## Phase 4.13.5: Bitwise Operations (6 handlers) [MEDIUM PRIORITY]

### **Implementation Target: 41/58 handlers (71%)**

#### **Handlers to Implement**
```cpp
OP_BITWISE_AND = 0x60,   // Pop b, pop a, push(a & b)
OP_BITWISE_OR = 0x61,    // Pop b, pop a, push(a | b)
OP_BITWISE_XOR = 0x62,   // Pop b, pop a, push(a ^ b)
OP_BITWISE_NOT = 0x63,   // Pop a, push(~a)
OP_SHIFT_LEFT = 0x64,    // Pop b, pop a, push(a << b)
OP_SHIFT_RIGHT = 0x65,   // Pop b, pop a, push(a >> b)
```

#### **Implementation with Shift Safety**
```cpp
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

vm_return_t ExecutionEngine_v2::handle_shift_right_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t b, a;  // b = shift amount, a = value to shift
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate shift amount (0-31 for 32-bit values)
    if (b < 0 || b >= 32) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    // Arithmetic right shift (preserves sign)
    int32_t result = a >> b;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_bitwise_not_impl(uint16_t immediate) noexcept {
    (void)immediate;
    int32_t a;
    if (!pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = ~a;

    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

---

## Phase 4.13.6: Arduino HAL Operations (11 handlers) [MEDIUM PRIORITY]

### **Implementation Target: 52/58 handlers (90%)**

#### **Handlers to Implement**
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

#### **GPIO Operations Implementation**
```cpp
vm_return_t ExecutionEngine_v2::handle_digital_write_impl(uint16_t immediate) noexcept {
    (void)immediate;  // Pin and value come from stack

    int32_t value, pin;
    if (!pop_protected(value) || !pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_PIN);
    }

    // Validate digital value (0 or 1)
    if (value < 0 || value > 1) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    // Use IOController interface
    if (!io_->digital_write(static_cast<uint8_t>(pin), static_cast<uint8_t>(value))) {
        return vm_return_t::error(VM_ERROR_IO_OPERATION_FAILED);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_digital_read_impl(uint16_t immediate) noexcept {
    (void)immediate;

    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_PIN);
    }

    uint8_t value;
    if (!io_->digital_read(static_cast<uint8_t>(pin), value)) {
        return vm_return_t::error(VM_ERROR_IO_OPERATION_FAILED);
    }

    if (!push_protected(static_cast<int32_t>(value))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_pin_mode_impl(uint16_t immediate) noexcept {
    (void)immediate;

    int32_t mode, pin;
    if (!pop_protected(mode) || !pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_PIN);
    }

    // Validate mode (INPUT=0, OUTPUT=1, INPUT_PULLUP=2)
    if (mode < 0 || mode > 3) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    if (!io_->pin_mode(static_cast<uint8_t>(pin), static_cast<uint8_t>(mode))) {
        return vm_return_t::error(VM_ERROR_IO_OPERATION_FAILED);
    }

    return vm_return_t::success();
}
```

#### **Timing Operations Implementation**
```cpp
vm_return_t ExecutionEngine_v2::handle_delay_impl(uint16_t immediate) noexcept {
    (void)immediate;

    int32_t nanoseconds;
    if (!pop_protected(nanoseconds)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (nanoseconds < 0) {
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    io_->delay_nanoseconds(static_cast<uint32_t>(nanoseconds));

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_millis_impl(uint16_t immediate) noexcept {
    (void)immediate;

    uint32_t timestamp = io_->millis();

    if (!push_protected(static_cast<int32_t>(timestamp))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_micros_impl(uint16_t immediate) noexcept {
    (void)immediate;

    uint32_t timestamp = io_->micros();

    if (!push_protected(static_cast<int32_t>(timestamp))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}
```

#### **Printf Implementation**
```cpp
vm_return_t ExecutionEngine_v2::handle_printf_impl(uint16_t immediate) noexcept {
    uint8_t string_id = static_cast<uint8_t>(immediate);

    // Pop argument count from stack
    int32_t arg_count_value;
    if (!pop_protected(arg_count_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (arg_count_value < 0 || arg_count_value > 8) {  // Max 8 printf args
        return vm_return_t::error(VM_ERROR_INVALID_OPERAND);
    }

    uint8_t arg_count = static_cast<uint8_t>(arg_count_value);

    // Pop arguments from stack (in reverse order)
    int32_t args[8];
    for (uint8_t i = 0; i < arg_count; i++) {
        if (!pop_protected(args[arg_count - 1 - i])) {  // Reverse order
            return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
        }
    }

    if (!io_->vm_printf(string_id, args, arg_count)) {
        return vm_return_t::error(VM_ERROR_IO_OPERATION_FAILED);
    }

    return vm_return_t::success();
}
```

---

## Phase 4.13.7: Multimedia Operations (6 handlers) [LOW PRIORITY]

### **Implementation Target: 58/58 handlers (100%)**

#### **Handlers to Implement**
```cpp
OP_DISPLAY_CLEAR = 0x70, // display_clear() - Clear 128x32 OLED buffer
OP_DISPLAY_TEXT = 0x71,  // display_text(x,y,text) - Draw text at position
OP_DISPLAY_UPDATE = 0x72,// display_update() - Flush buffer to OLED hardware
OP_BUTTON_READ = 0x73,   // button_read() -> 5-bit button matrix (PC0-PC4)
OP_LED_MORSE = 0x74,     // led_morse(pattern) - Start non-blocking LED morse
```

**Note**: These require IOController extension for display and LED functionality.

---

## Comprehensive Test Strategy

### **GT Lite Test Suite Expansion**

#### **Test File Structure**
```bash
tests/test_registry/lite_src/test_lite_control_flow.c       # 6 tests
tests/test_registry/lite_src/test_lite_comparisons_ext.c    # 16 tests
tests/test_registry/lite_src/test_lite_logical.c           # 9 tests
tests/test_registry/lite_src/test_lite_memory.c            # 15 tests
tests/test_registry/lite_src/test_lite_bitwise.c           # 12 tests
tests/test_registry/lite_src/test_lite_arduino_hal.c       # 20 tests
tests/test_registry/lite_src/test_lite_multimedia.c        # 10 tests

tests/test_registry/lite_data/test_control_flow.c
tests/test_registry/lite_data/test_comparisons_ext.c
tests/test_registry/lite_data/test_logical.c
tests/test_registry/lite_data/test_memory.c
tests/test_registry/lite_data/test_bitwise.c
tests/test_registry/lite_data/test_arduino_hal.c
tests/test_registry/lite_data/test_multimedia.c
```

#### **Test Coverage Matrix**
```
Phase 4.13.1: Control Flow        →  6 tests  → Total:  25 tests
Phase 4.13.2: Extended Comparisons → 16 tests  → Total:  41 tests
Phase 4.13.3: Logical Operations  →  9 tests  → Total:  50 tests
Phase 4.13.4: Memory Operations   → 15 tests  → Total:  65 tests
Phase 4.13.5: Bitwise Operations  → 12 tests  → Total:  77 tests
Phase 4.13.6: Arduino HAL         → 20 tests  → Total:  97 tests
Phase 4.13.7: Multimedia          → 10 tests  → Total: 107 tests

GRAND TOTAL: 107 GT Lite tests covering all 58 VMOpcode handlers
```

### **Test Validation Protocol**

#### **Regression Testing at Each Phase**
```bash
# After each phase implementation, run full regression
USE_EXECUTION_ENGINE_V2=1 make test_lite_stack        # Baseline: 4/4 tests
USE_EXECUTION_ENGINE_V2=1 make test_lite_comparison   # Baseline: 9/9 tests
USE_EXECUTION_ENGINE_V2=1 make test_lite_arithmetic   # Baseline: 6/6 tests
USE_EXECUTION_ENGINE_V2=1 make test_lite_control_flow # New: 6/6 tests

# Comprehensive suite validation
./run_all_gt_lite_tests.sh  # Script to run all test suites
```

#### **Performance Benchmarking**
```bash
# Measure binary search performance with expanded handler table
time USE_EXECUTION_ENGINE_V2=1 ./test_lite_comprehensive
# Expected: O(log 58) ≈ 5.9 comparisons vs current O(log 14) ≈ 3.8
# Memory usage: ~460 bytes vs 1KB traditional (55% reduction maintained)
```

#### **Memory Integration Testing**
```bash
# Validate MemoryManager operations with ExecutionEngine_v2
USE_EXECUTION_ENGINE_V2=1 make test_lite_memory
./test_lite_memory --verbose

# Verify static VMMemoryContext integration
# Test global variables: 0-63 (64 total)
# Test arrays: 0-15 (16 total), max 1024 elements each
# Test memory bounds checking and error handling
```

---

## Error Handling Expansion

### **New VM Error Codes Required**
```cpp
// Add to vm_errors.h
VM_ERROR_INVALID_JUMP = 20,              // Jump address out of bounds
VM_ERROR_INVALID_PIN = 21,               // GPIO pin number invalid
VM_ERROR_IO_OPERATION_FAILED = 22,       // Hardware I/O operation failed
VM_ERROR_INVALID_MEMORY_ACCESS = 23,     // Global/array access out of bounds
VM_ERROR_MEMORY_ALLOCATION_FAILED = 24,  // Array creation failed
VM_ERROR_INVALID_OPERAND = 25,           // Parameter validation failed
```

### **Error Propagation Testing**
Each new handler must include error condition tests:
- Stack underflow (insufficient operands)
- Stack overflow (result storage)
- Parameter validation (ranges, bounds)
- Resource availability (memory, I/O)
- Hardware failure simulation

---

## Implementation Timeline

### **Sprint Planning (3-4 weeks total)**

#### **Week 1: Critical Operations**
- **Day 1-2**: Phase 4.13.1 Control Flow (3 handlers + 6 tests)
- **Day 3-4**: Phase 4.13.2 Extended Comparisons (8 handlers + 16 tests)
- **Day 5**: Phase 4.13.3 Logical Operations (3 handlers + 9 tests)
- **Result**: 28/58 handlers (48%), 50 total tests

#### **Week 2: Core Functionality**
- **Day 1-3**: Phase 4.13.4 Memory Operations (7 handlers + 15 tests)
- **Day 4-5**: Phase 4.13.5 Bitwise Operations (6 handlers + 12 tests)
- **Result**: 41/58 handlers (71%), 77 total tests

#### **Week 3: Hardware Integration**
- **Day 1-3**: Phase 4.13.6 Arduino HAL (11 handlers + 20 tests)
- **Day 4-5**: Phase 4.13.7 Multimedia (6 handlers + 10 tests)
- **Result**: 58/58 handlers (100%), 107 total tests

#### **Week 4: Validation & Documentation**
- **Day 1-2**: Comprehensive regression testing
- **Day 3**: Performance optimization and benchmarking
- **Day 4**: Documentation and migration guide updates
- **Day 5**: STM32G474 hardware validation

---

## Success Criteria

### **Functional Requirements**
- [ ] All 58 VMOpcode handlers implemented with consistent interface
- [ ] 107 GT Lite tests covering success and error conditions
- [ ] Zero regressions in existing 19 baseline tests
- [ ] Complete MemoryManager and IOController integration

### **Performance Requirements**
- [ ] Binary search maintains O(log n) lookup (≤6 comparisons for 58 handlers)
- [ ] Sparse table memory usage ≤500 bytes (vs 1KB traditional)
- [ ] No execution speed degradation beyond logarithmic scaling
- [ ] STM32G474 memory constraints respected (128KB flash, 32KB RAM)

### **Quality Requirements**
- [ ] Comprehensive error handling with unified error codes
- [ ] Consistent handler interface patterns across all opcodes
- [ ] GT Lite rapid testing maintains 300x performance advantage
- [ ] Production-ready documentation and integration guides

### **Production Readiness**
- [ ] Hardware validation on STM32G474 WeAct Studio CoreBoard
- [ ] Real-time performance characterization
- [ ] Memory safety validation with bounds checking
- [ ] Phase 5.0 cooperative scheduling foundation complete

---

**IMPLEMENTATION READY**: This plan provides systematic 7-phase implementation of all 44 remaining handlers with comprehensive test coverage, establishing complete production-ready embedded hypervisor for Phase 5.0 cooperative task scheduling.

**IMMEDIATE NEXT ACTION**: Begin Phase 4.13.1 - Implement control flow operations (JMP, JMP_TRUE, JMP_FALSE) with 6-test GT Lite validation suite to enable loops and conditionals in guest programs.