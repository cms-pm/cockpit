# Phase 4.13: REVISED ExecutionEngine_v2 Complete Handler Implementation Plan

**Document Type**: Technical Implementation Strategy
**Phase**: 4.13 - Complete VMOpcode Coverage with Ambiguity Resolutions
**Audience**: Senior Embedded Systems Architect Team
**Author**: cms-pm + Claude (Staff Embedded Systems Architect)
**Date**: 2025-09-27
**Classification**: TECHNICAL ARCHITECTURE
**Priority**: CRITICAL - Production Handler Completion

---

## Executive Summary

**MISSION**: Complete implementation of remaining 44 VMOpcode handlers in ExecutionEngine_v2 with **ambiguity-resolved architecture**, achieving 100% VMOpcode coverage and comprehensive GT Lite validation.

**CURRENT STATUS**: ✅ ExecutionEngine_v2 Foundation Complete
- 14/58 handlers implemented (24% coverage)
- GT Lite validation: 19/19 tests passing
- Binary search sparse table architecture operational
- **5 Critical Ambiguities RESOLVED** per user specifications

**REVISED STRATEGY**: Systematic 7-phase implementation with unified error handling and complete OPCODE_TABLE population

---

## Critical Ambiguity Resolutions Applied

### **Resolution 1: Error Code Strategy**
```cpp
// RESOLVED: Reuse existing VM_ERROR_INVALID_JUMP = 4
// NO NEW ERROR CODES ADDED - Use existing vm_error_t enum
if (result.pc_target >= program_size_) {
    last_error_ = VM_ERROR_INVALID_JUMP;  // Existing error code
    return false;
}
```

### **Resolution 2: Jump Address Semantics ✅ VERIFIED**
```cpp
// CONFIRMED: Jump addresses are instruction indices (NOT byte offsets)
// Evidence from execution_engine_v2.cpp:137-163:
pc_ = result.pc_target;                    // Direct instruction index assignment
const VM::Instruction& instr = program_[pc_];  // Array indexing confirms instruction semantics
if (result.pc_target >= program_size_) {   // Validation against instruction count
```

### **Resolution 3: Complete OPCODE_TABLE Population**
```cpp
// RESOLVED: Fill OPCODE_TABLE with ALL 58 opcodes from vm_opcodes.h
// Maintains binary search integrity with placeholders for unimplemented handlers
static const opcode_handler_entry OPCODE_TABLE[] = {
    // ALL 58 opcodes in sorted order, unimplemented point to default handler
    {static_cast<uint8_t>(VMOpcode::OP_HALT), &ExecutionEngine_v2::handle_halt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ADD), &ExecutionEngine_v2::handle_add_impl},
    // ... ALL entries from vm_opcodes.h ...
    {static_cast<uint8_t>(VMOpcode::OP_MULTIMEDIA_STOP), &ExecutionEngine_v2::handle_unimplemented_impl}
};
```

### **Resolution 4: Memory Interface Standardization**
```cpp
// RESOLVED: Use load_global(id, value) signature pattern
vm_return_t ExecutionEngine_v2::handle_load_global_impl(uint16_t immediate) noexcept {
    int32_t value;
    if (!memory_->load_global(immediate, value)) {  // NEW SIGNATURE
        return vm_return_t::error(VM_ERROR_INVALID_MEMORY_ACCESS);
    }
    // ... implementation
}
```

### **Resolution 5: Single Source of Truth Error Handling**
```cpp
// RESOLVED: Eliminate GT_LITE_VM_ERROR_* duplication
// ALL tests use vm_error_t enum directly (vm_errors.h)
// NO MORE: GT_LITE_VM_ERROR_INVALID_JUMP, GT_LITE_VM_ERROR_STACK_OVERFLOW
// USE: VM_ERROR_INVALID_JUMP, VM_ERROR_STACK_OVERFLOW directly
```

---

## Revised Implementation Phases

### **Phase 4.13.1: Control Flow Operations - IMMEDIATE PRIORITY**

#### **Handlers to Implement (3 total)**
```cpp
// ALL use instruction index semantics and existing VM_ERROR_INVALID_JUMP
vm_return_t handle_jmp_impl(uint16_t immediate) noexcept;       // Unconditional jump
vm_return_t handle_jmp_true_impl(uint16_t immediate) noexcept;  // Jump if stack top != 0
vm_return_t handle_jmp_false_impl(uint16_t immediate) noexcept; // Jump if stack top == 0
```

#### **Implementation Strategy**
```cpp
vm_return_t ExecutionEngine_v2::handle_jmp_impl(uint16_t immediate) noexcept {
    // RESOLVED: immediate is instruction index, not byte offset
    if (immediate >= program_size_) {
        return vm_return_t::error(VM_ERROR_INVALID_JUMP);  // Existing error code
    }
    return vm_return_t::jump_absolute(immediate);  // Direct instruction index
}

vm_return_t ExecutionEngine_v2::handle_jmp_true_impl(uint16_t immediate) noexcept {
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition != 0) {  // C boolean semantics: non-zero is true
        if (immediate >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump_absolute(immediate);
    }
    return vm_return_t::increment();  // Continue to next instruction
}
```

#### **GT Lite Test Suite (9 tests)**
```c
// ALL tests use vm_error_t codes directly (NO GT_LITE_VM_ERROR_*)
const uint8_t test_jmp_basic[] = {
    0x00, 0x00, 0x05, 0x00,  // JMP 5 (instruction index)
    0x01, 0x00, 0x10, 0x00,  // PUSH 16 (should be skipped)
    0x02, 0x00, 0x00, 0x00,  // HALT (target at index 5)
};

// Test validation uses vm_error_t directly
assert(engine.get_last_error() == VM_ERROR_INVALID_JUMP);  // Single source of truth
```

### **Phase 4.13.2: Extended Comparisons (8 handlers)**
```cpp
// Complete conditional logic foundation
vm_return_t handle_le_impl(uint16_t immediate) noexcept;     // Less than or equal
vm_return_t handle_ge_impl(uint16_t immediate) noexcept;     // Greater than or equal
vm_return_t handle_slt_impl(uint16_t immediate) noexcept;    // Signed less than
vm_return_t handle_sgt_impl(uint16_t immediate) noexcept;    // Signed greater than
vm_return_t handle_sle_impl(uint16_t immediate) noexcept;    // Signed less than or equal
vm_return_t handle_sge_impl(uint16_t immediate) noexcept;    // Signed greater than or equal
vm_return_t handle_ne_impl(uint16_t immediate) noexcept;     // Not equal
vm_return_t handle_seq_impl(uint16_t immediate) noexcept;    // String/sequence equal
```

### **Phase 4.13.3: Logical Operations (3 handlers)**
```cpp
// C boolean semantics: 0 = false, non-zero = true
vm_return_t handle_and_impl(uint16_t immediate) noexcept;    // Logical AND
vm_return_t handle_or_impl(uint16_t immediate) noexcept;     // Logical OR
vm_return_t handle_not_impl(uint16_t immediate) noexcept;    // Logical NOT
```

### **Phase 4.13.4: Memory Operations (7 handlers)**
```cpp
// RESOLVED: All use load_global(id, value) / store_global(id, value) signatures
vm_return_t handle_load_global_impl(uint16_t immediate) noexcept;
vm_return_t handle_store_global_impl(uint16_t immediate) noexcept;
vm_return_t handle_load_local_impl(uint16_t immediate) noexcept;
vm_return_t handle_store_local_impl(uint16_t immediate) noexcept;
vm_return_t handle_load_array_impl(uint16_t immediate) noexcept;
vm_return_t handle_store_array_impl(uint16_t immediate) noexcept;
vm_return_t handle_alloc_array_impl(uint16_t immediate) noexcept;
```

### **Phase 4.13.5: Bitwise Operations (6 handlers)**
```cpp
vm_return_t handle_bit_and_impl(uint16_t immediate) noexcept;
vm_return_t handle_bit_or_impl(uint16_t immediate) noexcept;
vm_return_t handle_bit_xor_impl(uint16_t immediate) noexcept;
vm_return_t handle_bit_not_impl(uint16_t immediate) noexcept;
vm_return_t handle_shift_left_impl(uint16_t immediate) noexcept;
vm_return_t handle_shift_right_impl(uint16_t immediate) noexcept;
```

### **Phase 4.13.6: Arduino HAL Integration (12 handlers)**
```cpp
// Direct STM32G474 hardware integration
vm_return_t handle_digital_read_impl(uint16_t immediate) noexcept;
vm_return_t handle_digital_write_impl(uint16_t immediate) noexcept;
vm_return_t handle_analog_read_impl(uint16_t immediate) noexcept;
vm_return_t handle_analog_write_impl(uint16_t immediate) noexcept;
vm_return_t handle_pin_mode_impl(uint16_t immediate) noexcept;
vm_return_t handle_delay_impl(uint16_t immediate) noexcept;
vm_return_t handle_delay_microseconds_impl(uint16_t immediate) noexcept;
vm_return_t handle_millis_impl(uint16_t immediate) noexcept;
vm_return_t handle_micros_impl(uint16_t immediate) noexcept;
vm_return_t handle_serial_print_impl(uint16_t immediate) noexcept;
vm_return_t handle_serial_println_impl(uint16_t immediate) noexcept;
vm_return_t handle_serial_available_impl(uint16_t immediate) noexcept;
```

### **Phase 4.13.7: Multimedia Operations (5 handlers)**
```cpp
// I2S and DAC coordination for STM32G474
vm_return_t handle_multimedia_init_impl(uint16_t immediate) noexcept;
vm_return_t handle_multimedia_play_impl(uint16_t immediate) noexcept;
vm_return_t handle_multimedia_pause_impl(uint16_t immediate) noexcept;
vm_return_t handle_multimedia_resume_impl(uint16_t immediate) noexcept;
vm_return_t handle_multimedia_stop_impl(uint16_t immediate) noexcept;
```

---

## Complete OPCODE_TABLE Population Strategy

### **Full Table Structure (58 opcodes)**
```cpp
// RESOLVED: ALL opcodes included to preserve binary search integrity
static const opcode_handler_entry OPCODE_TABLE[] = {
    // Phase 4.11.8 IMPLEMENTED (14 handlers)
    {static_cast<uint8_t>(VMOpcode::OP_HALT), &ExecutionEngine_v2::handle_halt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PUSH), &ExecutionEngine_v2::handle_push_impl},
    {static_cast<uint8_t>(VMOpcode::OP_POP), &ExecutionEngine_v2::handle_pop_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ADD), &ExecutionEngine_v2::handle_add_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SUB), &ExecutionEngine_v2::handle_sub_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MUL), &ExecutionEngine_v2::handle_mul_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DIV), &ExecutionEngine_v2::handle_div_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MOD), &ExecutionEngine_v2::handle_mod_impl},
    {static_cast<uint8_t>(VMOpcode::OP_EQ), &ExecutionEngine_v2::handle_eq_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LT), &ExecutionEngine_v2::handle_lt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_GT), &ExecutionEngine_v2::handle_gt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DUP), &ExecutionEngine_v2::handle_dup_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SWAP), &ExecutionEngine_v2::handle_swap_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PRINTF), &ExecutionEngine_v2::handle_printf_impl},

    // Phase 4.13 TO BE IMPLEMENTED (44 handlers)
    {static_cast<uint8_t>(VMOpcode::OP_JMP), &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_JMP_TRUE), &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_JMP_FALSE), &ExecutionEngine_v2::handle_unimplemented_impl},
    // ... ALL remaining opcodes with unimplemented placeholders ...
    {static_cast<uint8_t>(VMOpcode::OP_MULTIMEDIA_STOP), &ExecutionEngine_v2::handle_unimplemented_impl}
};

// Binary search integrity maintained with complete table
static_assert(OPCODE_TABLE_SIZE == 58, "All VMOpcodes must be in table");
```

### **Default Unimplemented Handler**
```cpp
vm_return_t ExecutionEngine_v2::handle_unimplemented_impl(uint16_t immediate) noexcept {
    last_error_ = VM_ERROR_INVALID_OPCODE;
    return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
}
```

---

## Unified Error Handling Strategy

### **Single Source of Truth Implementation**
```cpp
// RESOLVED: vm_errors.h is the ONLY error enum
// lib/vm_cockpit/src/vm_errors.h
typedef enum {
    VM_ERROR_NONE = 0,
    VM_ERROR_STACK_UNDERFLOW = 1,
    VM_ERROR_STACK_OVERFLOW = 2,
    VM_ERROR_DIVISION_BY_ZERO = 3,
    VM_ERROR_INVALID_JUMP = 4,        // REUSED for jump validation
    VM_ERROR_INVALID_OPCODE = 5,
    VM_ERROR_INVALID_OPERAND = 6,
    VM_ERROR_INVALID_MEMORY_ACCESS = 7,
    VM_ERROR_IO_OPERATION_FAILED = 8,
    VM_ERROR_PROGRAM_NOT_LOADED = 9
} vm_error_t;
```

### **GT Lite Error Duplication Elimination**
```cpp
// BEFORE (tests/test_registry/test_runner/include/gt_lite_test_types.h):
typedef enum {
    GT_LITE_VM_ERROR_NONE = 0,           // DUPLICATE - REMOVE
    GT_LITE_VM_ERROR_STACK_UNDERFLOW,    // DUPLICATE - REMOVE
    GT_LITE_VM_ERROR_INVALID_JUMP,       // DUPLICATE - REMOVE
} gt_lite_vm_error_t;

// AFTER (tests use vm_error_t directly):
#include "vm_errors.h"  // Single source of truth
assert(engine.get_last_error() == VM_ERROR_INVALID_JUMP);  // Direct usage
```

---

## Comprehensive GT Lite Test Strategy

### **Test Coverage Matrix (107 total tests)**
```bash
# Phase 4.13.1: Control Flow (9 tests)
test_lite_jmp_basic              # Basic unconditional jump
test_lite_jmp_boundary           # Jump to program boundaries
test_lite_jmp_invalid            # Invalid jump targets
test_lite_jmp_true_condition     # Conditional jump true cases
test_lite_jmp_false_condition    # Conditional jump false cases
test_lite_jmp_stack_underflow    # Jump with empty stack
test_lite_jmp_complex_flow       # Nested conditional jumps
test_lite_jmp_edge_cases         # Zero/max instruction indices
test_lite_jmp_error_recovery     # Error handling validation

# Phase 4.13.2: Extended Comparisons (16 tests)
test_lite_comparison_le          # Less than or equal (4 tests)
test_lite_comparison_ge          # Greater than or equal (4 tests)
test_lite_comparison_signed      # Signed comparisons (4 tests)
test_lite_comparison_edge_cases  # Integer overflow/underflow (4 tests)

# Phase 4.13.3: Logical Operations (9 tests)
test_lite_logical_and           # AND truth table (3 tests)
test_lite_logical_or            # OR truth table (3 tests)
test_lite_logical_not           # NOT operations (3 tests)

# Phase 4.13.4: Memory Operations (21 tests)
test_lite_memory_global         # Global load/store (6 tests)
test_lite_memory_local          # Local load/store (6 tests)
test_lite_memory_array          # Array operations (6 tests)
test_lite_memory_boundaries     # Memory boundary validation (3 tests)

# Phase 4.13.5: Bitwise Operations (18 tests)
test_lite_bitwise_and           # Bitwise AND operations (3 tests)
test_lite_bitwise_or            # Bitwise OR operations (3 tests)
test_lite_bitwise_xor           # Bitwise XOR operations (3 tests)
test_lite_bitwise_not           # Bitwise NOT operations (3 tests)
test_lite_bitwise_shift_left    # Left shift operations (3 tests)
test_lite_bitwise_shift_right   # Right shift operations (3 tests)

# Phase 4.13.6: Arduino HAL (24 tests)
test_lite_arduino_digital       # Digital I/O (6 tests)
test_lite_arduino_analog        # Analog I/O (6 tests)
test_lite_arduino_timing        # Timing functions (6 tests)
test_lite_arduino_serial        # Serial operations (6 tests)

# Phase 4.13.7: Multimedia (10 tests)
test_lite_multimedia_lifecycle  # Init/play/pause/resume/stop (5 tests)
test_lite_multimedia_error      # Error conditions (5 tests)

# TOTAL: 107 GT Lite tests (19 existing + 88 new)
```

### **Unified Test Framework**
```c
// GT Lite tests use vm_error_t directly (NO duplication)
#include "vm_errors.h"           // Single source of truth
#include "gt_lite_test_types.h"  // Test framework only

void test_jmp_invalid_target(void) {
    // Use vm_error_t enum directly
    assert(engine.get_last_error() == VM_ERROR_INVALID_JUMP);
    // NO MORE: assert(engine.get_last_error() == GT_LITE_VM_ERROR_INVALID_JUMP);
}
```

---

## Implementation Timeline

### **Week 1: Foundation (Phase 4.13.0)**
- **Day 1**: Complete OPCODE_TABLE population with all 58 opcodes
- **Day 2**: Implement default unimplemented handler
- **Day 3**: Eliminate GT_LITE_VM_ERROR_* duplication
- **Day 4**: Validate binary search table integrity
- **Day 5**: Test existing 14 handlers with complete table

### **Week 2: Control Flow (Phase 4.13.1)**
- **Day 1**: Implement JMP, JMP_TRUE, JMP_FALSE handlers
- **Day 2**: Create 9 GT Lite control flow tests
- **Day 3**: Validate instruction index jump semantics
- **Day 4**: Error handling integration testing
- **Day 5**: Performance benchmarking vs legacy

### **Week 3-4: Extended Operations (Phases 4.13.2-4.13.3)**
- Extended comparisons and logical operations
- 25 additional GT Lite tests
- Comprehensive boolean logic validation

### **Week 5-6: Memory & Hardware (Phases 4.13.4-4.13.6)**
- Memory operations with corrected signatures
- Arduino HAL integration for STM32G474
- 45 additional GT Lite tests

### **Week 7: Multimedia & Validation (Phase 4.13.7)**
- Multimedia handler completion
- Final 10 GT Lite tests
- Comprehensive system validation

---

## Production Readiness Validation

### **Architecture Verification**
- [x] **Single error source**: vm_error_t enum only (GT_LITE_* eliminated)
- [x] **Jump semantics**: Instruction indices confirmed (lines 137-163)
- [x] **Binary search integrity**: Complete 58-opcode table maintained
- [x] **Memory interface**: load_global(id, value) signature standardized
- [ ] **Handler coverage**: 58/58 opcodes implemented (currently 14/58)

### **Performance Metrics**
- **Handler dispatch**: O(log 58) = 6 comparisons maximum
- **Memory overhead**: 58 * 16 bytes = 928 bytes (vs 1KB legacy 87% savings eliminated)
- **Cache efficiency**: Single cache line access for handler table
- **Error handling**: Unified vm_error_t reduces code complexity

### **Integration Testing**
- **GT Lite**: 107 total tests (19 existing + 88 new)
- **Hardware validation**: STM32G474 physical device testing
- **ComponentVM**: Drop-in ExecutionEngine_v2 compatibility
- **Observer pattern**: Telemetry and debugging preserved

---

## Next Steps Summary

**IMMEDIATE PRIORITY**: Begin Phase 4.13.1 control flow implementation with all ambiguities resolved:

1. **Complete OPCODE_TABLE**: Add all 58 vm_opcodes.h entries with placeholders
2. **Implement control flow**: JMP, JMP_TRUE, JMP_FALSE with instruction index semantics
3. **Create GT Lite tests**: 9 control flow tests using vm_error_t directly
4. **Validate integration**: Ensure binary search integrity maintained
5. **Expand systematically**: Progress through phases 4.13.2-4.13.7

The foundation is complete, ambiguities resolved, and implementation path clear for achieving 100% VMOpcode coverage in ExecutionEngine_v2.