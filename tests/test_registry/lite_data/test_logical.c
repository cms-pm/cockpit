#include "../test_runner/include/gt_lite_test_types.h"

// Phase 4.13.3: Logical Operations GT Lite Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)
// C boolean semantics: 0 = false, non-zero = true

// Test 1: AND - true && true = true (1 && 5 = true)
static const uint8_t and_true_true_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (true)
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (true)
    0x40, 0x00, 0x00, 0x00,  // AND (1 && 5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 2: AND - true && false = false (1 && 0 = false)
static const uint8_t and_true_false_bytecode[] = {
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (true)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x40, 0x00, 0x00, 0x00,  // AND (1 && 0)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: AND - false && true = false (0 && 7 = false)
static const uint8_t and_false_true_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x01, 0x00, 0x07, 0x00,  // PUSH 7 (true)
    0x40, 0x00, 0x00, 0x00,  // AND (0 && 7)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 4: AND - false && false = false (0 && 0 = false)
static const uint8_t and_false_false_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x40, 0x00, 0x00, 0x00,  // AND (0 && 0)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: OR - true || true = true (3 || 9 = true)
static const uint8_t or_true_true_bytecode[] = {
    0x01, 0x00, 0x03, 0x00,  // PUSH 3 (true)
    0x01, 0x00, 0x09, 0x00,  // PUSH 9 (true)
    0x41, 0x00, 0x00, 0x00,  // OR (3 || 9)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 6: OR - true || false = true (2 || 0 = true)
static const uint8_t or_true_false_bytecode[] = {
    0x01, 0x00, 0x02, 0x00,  // PUSH 2 (true)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x41, 0x00, 0x00, 0x00,  // OR (2 || 0)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 7: OR - false || true = true (0 || 4 = true)
static const uint8_t or_false_true_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x01, 0x00, 0x04, 0x00,  // PUSH 4 (true)
    0x41, 0x00, 0x00, 0x00,  // OR (0 || 4)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 8: OR - false || false = false (0 || 0 = false)
static const uint8_t or_false_false_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x41, 0x00, 0x00, 0x00,  // OR (0 || 0)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 9: NOT - !true = false (!42 = false)
static const uint8_t not_true_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH 42 (true)
    0x42, 0x00, 0x00, 0x00,  // NOT (!42)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 10: NOT - !false = true (!0 = true)
static const uint8_t not_false_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x42, 0x00, 0x00, 0x00,  // NOT (!0)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 11: Complex logical expression: !(5 && 0) || (3 && 7)
// Should be: !(true && false) || (true && true) = !false || true = true || true = true
static const uint8_t complex_logical_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (true)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (false)
    0x40, 0x00, 0x00, 0x00,  // AND (5 && 0 = false)
    0x42, 0x00, 0x00, 0x00,  // NOT (!false = true)
    0x01, 0x00, 0x03, 0x00,  // PUSH 3 (true)
    0x01, 0x00, 0x07, 0x00,  // PUSH 7 (true)
    0x40, 0x00, 0x00, 0x00,  // AND (3 && 7 = true)
    0x41, 0x00, 0x00, 0x00,  // OR (true || true = true)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 12: AND stack underflow test
static const uint8_t and_stack_underflow_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (only one value)
    0x40, 0x00, 0x00, 0x00,  // AND (needs two values - should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 13: OR stack underflow test
static const uint8_t or_stack_underflow_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (only one value)
    0x41, 0x00, 0x00, 0x00,  // OR (needs two values - should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 14: NOT stack underflow test
static const uint8_t not_stack_underflow_bytecode[] = {
    0x42, 0x00, 0x00, 0x00,  // NOT (needs one value - should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// GT Lite test cases array
static const gt_lite_test_t logical_tests[] = {
        {
            .test_name = "and_true_true",
            .bytecode = and_true_true_bytecode,
            .bytecode_size = sizeof(and_true_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "and_true_false",
            .bytecode = and_true_false_bytecode,
            .bytecode_size = sizeof(and_true_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "and_false_true",
            .bytecode = and_false_true_bytecode,
            .bytecode_size = sizeof(and_false_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "and_false_false",
            .bytecode = and_false_false_bytecode,
            .bytecode_size = sizeof(and_false_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "or_true_true",
            .bytecode = or_true_true_bytecode,
            .bytecode_size = sizeof(or_true_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "or_true_false",
            .bytecode = or_true_false_bytecode,
            .bytecode_size = sizeof(or_true_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "or_false_true",
            .bytecode = or_false_true_bytecode,
            .bytecode_size = sizeof(or_false_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "or_false_false",
            .bytecode = or_false_false_bytecode,
            .bytecode_size = sizeof(or_false_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "not_true",
            .bytecode = not_true_bytecode,
            .bytecode_size = sizeof(not_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "not_false",
            .bytecode = not_false_bytecode,
            .bytecode_size = sizeof(not_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "complex_logical",
            .bytecode = complex_logical_bytecode,
            .bytecode_size = sizeof(complex_logical_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "and_stack_underflow",
            .bytecode = and_stack_underflow_bytecode,
            .bytecode_size = sizeof(and_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "or_stack_underflow",
            .bytecode = or_stack_underflow_bytecode,
            .bytecode_size = sizeof(or_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "not_stack_underflow",
            .bytecode = not_stack_underflow_bytecode,
            .bytecode_size = sizeof(not_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        }
};

// GT Lite test suite structure
const gt_lite_test_suite_t logical_test_suite = {
    .suite_name = "logical",
    .test_count = 14,
    .tests = logical_tests
};