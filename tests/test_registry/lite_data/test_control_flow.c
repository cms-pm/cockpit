#include "../test_runner/include/gt_lite_test_types.h"

// Phase 4.13.1: Control Flow GT Lite Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)
// NOTE: Using existing GT_LITE_VM_ERROR_* for now, will eliminate duplication per user requirement

// Test 1: Basic unconditional jump forward
static const uint8_t jmp_forward_bytecode[] = {
    0x30, 0x00, 0x02, 0x00,  // JMP 2 (jump to instruction index 2)
    0x01, 0x00, 0x99, 0x00,  // PUSH 153 (should be skipped)
    0x00, 0x00, 0x00, 0x00   // HALT (target instruction)
};

// Test 2: Jump to invalid target (boundary check)
static const uint8_t jmp_invalid_target_bytecode[] = {
    0x30, 0x00, 0x05, 0x00,  // JMP 5 (invalid - only 2 instructions exist)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: JMP_TRUE with true condition (non-zero)
static const uint8_t jmp_true_taken_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (true condition)
    0x31, 0x00, 0x03, 0x00,  // JMP_TRUE 3 (jump to instruction 3)
    0x01, 0x00, 0x99, 0x00,  // PUSH 153 (should be skipped)
    0x00, 0x00, 0x00, 0x00   // HALT (target instruction)
};

// Test 4: JMP_TRUE with false condition (zero) - should not jump
static const uint8_t jmp_true_not_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false condition)
    0x31, 0x00, 0x04, 0x00,  // JMP_TRUE 4 (should not jump)
    0x01, 0x00, 0x42, 0x00,  // PUSH 66 (should execute)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: JMP_FALSE with false condition (zero) - should jump
static const uint8_t jmp_false_taken_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false condition)
    0x32, 0x00, 0x03, 0x00,  // JMP_FALSE 3 (jump to instruction 3)
    0x01, 0x00, 0x99, 0x00,  // PUSH 153 (should be skipped)
    0x00, 0x00, 0x00, 0x00   // HALT (target instruction)
};

// Test 6: JMP_FALSE with true condition (non-zero) - should not jump
static const uint8_t jmp_false_not_taken_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (true condition)
    0x32, 0x00, 0x04, 0x00,  // JMP_FALSE 4 (should not jump)
    0x01, 0x00, 0x42, 0x00,  // PUSH 66 (should execute)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 7: JMP_TRUE with empty stack - should fail with stack underflow
static const uint8_t jmp_true_stack_underflow_bytecode[] = {
    0x31, 0x00, 0x02, 0x00,  // JMP_TRUE 2 (no value on stack)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 8: JMP_FALSE with empty stack - should fail with stack underflow
static const uint8_t jmp_false_stack_underflow_bytecode[] = {
    0x32, 0x00, 0x02, 0x00,  // JMP_FALSE 2 (no value on stack)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 9: Simple forward jump chain
static const uint8_t control_flow_simple_bytecode[] = {
    0x01, 0x00, 0x03, 0x00,  // 0: PUSH 3
    0x30, 0x00, 0x03, 0x00,  // 1: JMP 3 (skip next instruction)
    0x01, 0x00, 0x99, 0x00,  // 2: PUSH 153 (should be skipped)
    0x01, 0x00, 0x42, 0x00,  // 3: PUSH 66 (executed)
    0x00, 0x00, 0x00, 0x00   // 4: HALT
};

// GT Lite test cases array
static const gt_lite_test_t control_flow_tests[] = {
        {
            .test_name = "jmp_forward",
            .bytecode = jmp_forward_bytecode,
            .bytecode_size = sizeof(jmp_forward_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_invalid_target",
            .bytecode = jmp_invalid_target_bytecode,
            .bytecode_size = sizeof(jmp_invalid_target_bytecode),
            .expected_error = VM_ERROR_PC_OUT_OF_BOUNDS,  // Maps to VM_ERROR_INVALID_JUMP
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_true_taken",
            .bytecode = jmp_true_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_taken_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_true_not_taken",
            .bytecode = jmp_true_not_taken_bytecode,
            .bytecode_size = sizeof(jmp_true_not_taken_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {66},
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_false_taken",
            .bytecode = jmp_false_taken_bytecode,
            .bytecode_size = sizeof(jmp_false_taken_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_false_not_taken",
            .bytecode = jmp_false_not_taken_bytecode,
            .bytecode_size = sizeof(jmp_false_not_taken_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {66},
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_true_stack_underflow",
            .bytecode = jmp_true_stack_underflow_bytecode,
            .bytecode_size = sizeof(jmp_true_stack_underflow_bytecode),
            .expected_error = VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "jmp_false_stack_underflow",
            .bytecode = jmp_false_stack_underflow_bytecode,
            .bytecode_size = sizeof(jmp_false_stack_underflow_bytecode),
            .expected_error = VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "control_flow_simple",
            .bytecode = control_flow_simple_bytecode,
            .bytecode_size = sizeof(control_flow_simple_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {3, 66},  // Stack: bottom to top
            .expected_stack_size = 2,
            .memory_address = 0,
            .expected_memory_value = 0
        }
};

// GT Lite test suite structure
const gt_lite_test_suite_t control_flow_test_suite = {
    .suite_name = "control_flow",
    .test_count = 9,
    .tests = control_flow_tests
};