# Phase 4.13: Immediate Action Plan - Complete Handler Implementation

**Document Type**: Implementation Action Plan
**Phase**: 4.13.0 - Ready to Execute
**Audience**: cms-pm (Implementation Ready)
**Author**: cms-pm + Claude (Staff Embedded Systems Architect)
**Date**: 2025-09-27
**Classification**: IMPLEMENTATION GUIDE
**Priority**: IMMEDIATE ACTION REQUIRED

---

## Executive Summary

**READY TO EXECUTE**: Complete plan for implementing remaining 44 VMOpcode handlers (58 total) with comprehensive GT Lite validation. ExecutionEngine_v2 foundation is solid and ready for systematic expansion.

**CURRENT STATUS**: 14/58 handlers (24%) ✅ VALIDATED
**TARGET**: 58/58 handlers (100%) with 107 GT Lite tests
**TIMELINE**: 3-4 weeks systematic implementation

---

## IMMEDIATE NEXT STEPS (Start Now)

### **Step 1: Begin Phase 4.13.1 - Control Flow Operations**

Control flow is **CRITICAL PRIORITY** because it enables:
- Loops in guest programs (while, for, do-while)
- Conditional execution (if, else, switch)
- Function calls and returns (already implemented)
- Complex program logic beyond linear execution

#### **Handlers to Implement First (3 handlers)**
```cpp
OP_JMP = 0x30,           // Unconditional jump
OP_JMP_TRUE = 0x31,      // Jump if top of stack is true
OP_JMP_FALSE = 0x32,     // Jump if top of stack is false
```

#### **Implementation Commands (Execute in Order)**

**1. Add Handler Declarations**
```bash
# Edit: lib/vm_cockpit/src/execution_engine/execution_engine_v2.h
# Add to public section after existing handlers:

vm_return_t handle_jmp_impl(uint16_t immediate) noexcept;
vm_return_t handle_jmp_true_impl(uint16_t immediate) noexcept;
vm_return_t handle_jmp_false_impl(uint16_t immediate) noexcept;
```

**2. Update OPCODE_TABLE**
```bash
# Edit: lib/vm_cockpit/src/execution_engine/execution_engine_v2.cpp
# Add to OPCODE_TABLE (MAINTAIN SORTED ORDER - critical for binary search):

static const opcode_handler_entry OPCODE_TABLE[] = {
    {static_cast<uint8_t>(VMOpcode::OP_HALT),       &ExecutionEngine_v2::handle_halt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PUSH),       &ExecutionEngine_v2::handle_push_impl},
    {static_cast<uint8_t>(VMOpcode::OP_POP),        &ExecutionEngine_v2::handle_pop_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ADD),        &ExecutionEngine_v2::handle_add_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SUB),        &ExecutionEngine_v2::handle_sub_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MUL),        &ExecutionEngine_v2::handle_mul_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DIV),        &ExecutionEngine_v2::handle_div_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MOD),        &ExecutionEngine_v2::handle_mod_impl},
    {static_cast<uint8_t>(VMOpcode::OP_CALL),       &ExecutionEngine_v2::handle_call_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RET),        &ExecutionEngine_v2::handle_ret_impl},
    {static_cast<uint8_t>(VMOpcode::OP_EQ),         &ExecutionEngine_v2::handle_eq_impl},
    {static_cast<uint8_t>(VMOpcode::OP_NE),         &ExecutionEngine_v2::handle_ne_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LT),         &ExecutionEngine_v2::handle_lt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_GT),         &ExecutionEngine_v2::handle_gt_impl},
    // ADD THESE THREE IN SORTED ORDER (0x30, 0x31, 0x32 come after 0x23):
    {static_cast<uint8_t>(VMOpcode::OP_JMP),        &ExecutionEngine_v2::handle_jmp_impl},
    {static_cast<uint8_t>(VMOpcode::OP_JMP_TRUE),   &ExecutionEngine_v2::handle_jmp_true_impl},
    {static_cast<uint8_t>(VMOpcode::OP_JMP_FALSE),  &ExecutionEngine_v2::handle_jmp_false_impl},
};
```

**3. Implement Handler Functions**
```bash
# Add to end of lib/vm_cockpit/src/execution_engine/execution_engine_v2.cpp
# Before the handle_invalid_opcode_impl function:

// ============================================================================
// CONTROL FLOW OPERATIONS - PHASE 4.13.1
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_jmp_impl(uint16_t immediate) noexcept {
    uint32_t target_address = static_cast<uint32_t>(immediate);

    // Validate jump target is within program bounds
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

**4. Add Required Error Code**
```bash
# Edit: lib/vm_cockpit/src/vm_errors.h
# Add new error code after existing ones:

VM_ERROR_INVALID_JUMP = 20,              // Jump address out of bounds
```

**5. Compile and Test Basic Implementation**
```bash
cd tests/test_registry/test_runner
USE_EXECUTION_ENGINE_V2=1 make test_lite_stack
./test_lite_stack  # Should still pass (4/4 tests) - regression check
```

### **Step 2: Create Control Flow GT Lite Test Suite**

#### **Create Test Data File**
```bash
# Create: tests/test_registry/lite_data/test_control_flow.c
```

**File Contents:**
```c
#include "../test_runner/include/gt_lite_test_types.h"
#include <assert.h>

// GT Lite Control Flow Operations Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: Unconditional jump forward
static const uint8_t jmp_forward_bytecode[] = {
    0x01, 0x00, 0x63, 0x00,  // PUSH(99) - opcode=0x01, immediate=99
    0x30, 0x00, 0x04, 0x00,  // JMP(4) - Jump to instruction 4
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - Should be skipped
    0x01, 0x00, 0x17, 0x00,  // PUSH(23) - Jump target (instruction 4)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 2: Conditional jump taken (true condition)
static const uint8_t jmp_true_taken_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH(1) - true condition
    0x31, 0x00, 0x04, 0x00,  // JMP_TRUE(4) - Should jump
    0x01, 0x00, 0x99, 0x00,  // PUSH(153) - Should be skipped
    0x01, 0x00, 0x55, 0x00,  // PUSH(85) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: Conditional jump not taken (false condition)
static const uint8_t jmp_true_not_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH(0) - false condition
    0x31, 0x00, 0x04, 0x00,  // JMP_TRUE(4) - Should NOT jump
    0x01, 0x00, 0x7B, 0x00,  // PUSH(123) - Should execute
    0x01, 0x00, 0x55, 0x00,  // PUSH(85) - Should execute
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 4: JMP_FALSE with false condition (jump taken)
static const uint8_t jmp_false_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH(0) - false condition
    0x32, 0x00, 0x04, 0x00,  // JMP_FALSE(4) - Should jump
    0x01, 0x00, 0x88, 0x00,  // PUSH(136) - Should be skipped
    0x01, 0x00, 0x77, 0x00,  // PUSH(119) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: Invalid jump address (error condition)
static const uint8_t jmp_invalid_address_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH(1)
    0x30, 0x00, 0xFF, 0x00,  // JMP(255) - Invalid address (beyond program)
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};

// Test 6: Stack underflow on conditional jump
static const uint8_t jmp_stack_underflow_bytecode[] = {
    0x31, 0x00, 0x02, 0x00,  // JMP_TRUE(2) - No condition on stack
    0x01, 0x00, 0x42, 0x00,  // PUSH(66) - Never reached
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Compile-time size validation
static_assert(sizeof(jmp_forward_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_forward_bytecode exceeds 100-element limit");
static_assert(sizeof(jmp_true_taken_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_true_taken_bytecode exceeds 100-element limit");
static_assert(sizeof(jmp_true_not_taken_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_true_not_taken_bytecode exceeds 100-element limit");
static_assert(sizeof(jmp_false_taken_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_false_taken_bytecode exceeds 100-element limit");
static_assert(sizeof(jmp_invalid_address_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_invalid_address_bytecode exceeds 100-element limit");
static_assert(sizeof(jmp_stack_underflow_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "jmp_stack_underflow_bytecode exceeds 100-element limit");

// GT Lite test suite definition
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
            .expected_stack_size = 2,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_true_taken",
            .bytecode = jmp_true_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {85},
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_true_not_taken",
            .bytecode = jmp_true_not_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_not_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {123, 85},
            .expected_stack_size = 2,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_false_taken",
            .bytecode = jmp_false_taken_bytecode,
            .bytecode_size = sizeof(jmp_false_taken_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {119},
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_invalid_address",
            .bytecode = jmp_invalid_address_bytecode,
            .bytecode_size = sizeof(jmp_invalid_address_bytecode),
            .expected_error = GT_LITE_VM_ERROR_INVALID_JUMP,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_stack_underflow",
            .bytecode = jmp_stack_underflow_bytecode,
            .bytecode_size = sizeof(jmp_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        }
    }
};
```

#### **Create Test Runner File**
```bash
# Create: tests/test_registry/lite_src/test_lite_control_flow.c
```

**File Contents:**
```c
#include "../test_runner/src/gt_lite_runner.h"
#include <stdio.h>
#include <string.h>

// Import test data
extern const gt_lite_test_suite_t control_flow_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Control flow operations test suite\n");
    printf("============================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: JMP, JMP_TRUE, JMP_FALSE operations with program control validation\n\n");

    return gt_lite_run_test_suite(&control_flow_test_suite, verbose);
}
```

#### **Update Makefile**
```bash
# Edit: tests/test_registry/test_runner/Makefile
# Add control flow test target:

test_lite_control_flow: ../lite_src/test_lite_control_flow.c ../lite_data/test_control_flow.c $(CORE_OBJS)
	@echo "Building GT Lite test: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
```

### **Step 3: Build and Test Control Flow Implementation**

```bash
cd tests/test_registry/test_runner

# Build control flow test
USE_EXECUTION_ENGINE_V2=1 make test_lite_control_flow

# Run control flow tests
./test_lite_control_flow --verbose

# Expected results:
# - jmp_forward: PASS (stack = [99, 23])
# - jmp_true_taken: PASS (stack = [85])
# - jmp_true_not_taken: PASS (stack = [123, 85])
# - jmp_false_taken: PASS (stack = [119])
# - jmp_invalid_address: PASS (error = VM_ERROR_INVALID_JUMP)
# - jmp_stack_underflow: PASS (error = VM_ERROR_STACK_UNDERFLOW)
#
# GT Lite Results: 6/6 tests passed
# ✓ GT Lite Control Flow Operations: ALL TESTS PASSED

# Regression check - ensure existing tests still pass
./test_lite_stack          # Should pass: 4/4 tests
./test_lite_comparison     # Should pass: 9/9 tests
./test_lite_arithmetic     # Should pass: 6/6 tests

# Total: 25/25 tests passing (19 existing + 6 new control flow)
```

### **Step 4: Commit Control Flow Implementation**

```bash
git add lib/vm_cockpit/src/execution_engine/execution_engine_v2.h
git add lib/vm_cockpit/src/execution_engine/execution_engine_v2.cpp
git add lib/vm_cockpit/src/vm_errors.h
git add tests/test_registry/lite_data/test_control_flow.c
git add tests/test_registry/lite_src/test_lite_control_flow.c
git add tests/test_registry/test_runner/Makefile

git commit -m "Phase 4.13.1: Control Flow Operations Complete - JMP, JMP_TRUE, JMP_FALSE

Implemented 3 critical control flow handlers enabling loops and conditionals:
- ✅ OP_JMP: Unconditional jump with target validation
- ✅ OP_JMP_TRUE: Conditional jump on true (non-zero) stack value
- ✅ OP_JMP_FALSE: Conditional jump on false (zero) stack value

GT Lite Validation: 6/6 control flow tests passing
- Jump forward execution control
- Conditional jump taken/not taken logic
- Error handling: invalid jump address, stack underflow

Handler Coverage: 17/58 (30%) - Critical milestone achieved
Total GT Lite Tests: 25/25 passing (19 baseline + 6 control flow)

🎯 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>"
```

---

## Next Phase Preview (After Control Flow)

### **Phase 4.13.2: Extended Comparisons (Priority 2)**

Once control flow is working, immediately proceed to extended comparisons:

#### **Quick Implementation Commands**
```bash
# Add 8 comparison handlers:
OP_LE, OP_GE (unsigned)
OP_EQ_SIGNED, OP_NE_SIGNED, OP_LT_SIGNED, OP_GT_SIGNED, OP_LE_SIGNED, OP_GE_SIGNED

# Pattern example:
vm_return_t ExecutionEngine_v2::handle_le_impl(uint16_t immediate) noexcept {
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }
    uint32_t ua = static_cast<uint32_t>(a);
    uint32_t ub = static_cast<uint32_t>(b);
    int32_t result = (ua <= ub) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }
    return vm_return_t::success();
}
```

---

## Key Implementation Principles

### **1. Always Maintain Binary Search Order**
```cpp
// CRITICAL: OPCODE_TABLE must remain sorted by opcode value
// Binary search will fail if order is wrong
// 0x00, 0x01, 0x02, 0x03, ... 0x20, 0x21, 0x22, 0x23, 0x30, 0x31, 0x32
```

### **2. Consistent Error Handling**
```cpp
// All handlers follow same pattern:
// 1. Validate parameters
// 2. Pop operands with protection
// 3. Perform operation
// 4. Push result with protection
// 5. Return appropriate vm_return_t
```

### **3. GT Lite Test Coverage**
```cpp
// Every handler needs:
// - Success case test
// - Error condition tests (stack underflow/overflow, invalid parameters)
// - Edge case tests (boundary values, special conditions)
```

### **4. Regression Testing**
```bash
# After every implementation phase:
./test_lite_stack          # Must remain 4/4 passing
./test_lite_comparison     # Must remain 9/9 passing
./test_lite_arithmetic     # Must remain 6/6 passing
# New test suite          # Must achieve 100% pass rate
```

---

## Success Metrics for Phase 4.13.1

### **Immediate Goals (Control Flow)**
- [ ] 3 control flow handlers implemented (JMP, JMP_TRUE, JMP_FALSE)
- [ ] 6 GT Lite tests passing (all control flow scenarios)
- [ ] Zero regressions in existing 19 tests
- [ ] Handler coverage: 17/58 (30%)

### **Validation Criteria**
- [ ] Binary search table maintains sorted order
- [ ] Jump address validation prevents segmentation faults
- [ ] Stack underflow/overflow protection working
- [ ] C boolean semantics correct (0=false, non-zero=true)

### **Performance Verification**
- [ ] Binary search: O(log 17) ≈ 4.1 comparisons (vs current 3.8)
- [ ] Memory usage: ~270 bytes (vs ~160 current, ~1KB traditional)
- [ ] Execution speed: No degradation beyond logarithmic scaling

**READY TO EXECUTE**: Begin implementation immediately with Phase 4.13.1 control flow operations.

**Command**: Start with Step 1 - Add handler declarations to execution_engine_v2.h