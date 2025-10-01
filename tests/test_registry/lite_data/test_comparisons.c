#include "../test_runner/include/gt_lite_test_types.h"

// Phase 4.13.2: Extended Comparisons GT Lite Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: LE (Less than or equal) - unsigned - should be true (5 <= 10)
static const uint8_t le_unsigned_true_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5
    0x01, 0x00, 0x0A, 0x00,  // PUSH 10
    0x24, 0x00, 0x00, 0x00,  // LE (5 <= 10)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 2: LE (Less than or equal) - unsigned - should be false (10 <= 5)
static const uint8_t le_unsigned_false_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH 10
    0x01, 0x00, 0x05, 0x00,  // PUSH 5
    0x24, 0x00, 0x00, 0x00,  // LE (10 <= 5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: GE (Greater than or equal) - unsigned - should be true (10 >= 5)
static const uint8_t ge_unsigned_true_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH 10
    0x01, 0x00, 0x05, 0x00,  // PUSH 5
    0x25, 0x00, 0x00, 0x00,  // GE (10 >= 5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 4: GE (Greater than or equal) - unsigned - should be false (5 >= 10)
static const uint8_t ge_unsigned_false_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5
    0x01, 0x00, 0x0A, 0x00,  // PUSH 10
    0x25, 0x00, 0x00, 0x00,  // GE (5 >= 10)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: EQ_SIGNED - should be true (-5 == -5)
static const uint8_t eq_signed_true_bytecode[] = {
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x26, 0x00, 0x00, 0x00,  // EQ_SIGNED (-5 == -5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 6: NE_SIGNED - should be true (-5 != -3)
static const uint8_t ne_signed_true_bytecode[] = {
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x01, 0x00, 0xFD, 0xFF,  // PUSH -3 (0xFFFFFFFD)
    0x27, 0x00, 0x00, 0x00,  // NE_SIGNED (-5 != -3)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 7: LT_SIGNED - should be true (-10 < -5)
static const uint8_t lt_signed_true_bytecode[] = {
    0x01, 0x00, 0xF6, 0xFF,  // PUSH -10 (0xFFFFFFF6)
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x28, 0x00, 0x00, 0x00,  // LT_SIGNED (-10 < -5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 8: GT_SIGNED - should be true (-5 > -10)
static const uint8_t gt_signed_true_bytecode[] = {
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x01, 0x00, 0xF6, 0xFF,  // PUSH -10 (0xFFFFFFF6)
    0x29, 0x00, 0x00, 0x00,  // GT_SIGNED (-5 > -10)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 9: LE_SIGNED - should be true (-10 <= -5)
static const uint8_t le_signed_true_bytecode[] = {
    0x01, 0x00, 0xF6, 0xFF,  // PUSH -10 (0xFFFFFFF6)
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x2A, 0x00, 0x00, 0x00,  // LE_SIGNED (-10 <= -5)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 10: GE_SIGNED - should be true (-5 >= -10)
static const uint8_t ge_signed_true_bytecode[] = {
    0x01, 0x00, 0xFB, 0xFF,  // PUSH -5 (0xFFFFFFFB)
    0x01, 0x00, 0xF6, 0xFF,  // PUSH -10 (0xFFFFFFF6)
    0x2B, 0x00, 0x00, 0x00,  // GE_SIGNED (-5 >= -10)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 11: Unsigned vs Signed behavior difference (large unsigned vs negative)
static const uint8_t unsigned_vs_signed_bytecode[] = {
    0x01, 0x00, 0xFF, 0xFF,  // PUSH 0xFFFF (65535 unsigned, -1 signed)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1
    0x22, 0x00, 0x00, 0x00,  // LT unsigned (65535 < 1 = false)
    0x01, 0x00, 0xFF, 0xFF,  // PUSH 0xFFFF again
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 again
    0x28, 0x00, 0x00, 0x00,  // LT_SIGNED (-1 < 1 = true)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 12: Stack underflow test
static const uint8_t comparison_stack_underflow_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (only one value)
    0x24, 0x00, 0x00, 0x00,  // LE (needs two values - should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// GT Lite test cases array
static const gt_lite_test_t comparisons_tests[] = {
        {
            .test_name = "le_unsigned_true",
            .bytecode = le_unsigned_true_bytecode,
            .bytecode_size = sizeof(le_unsigned_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "le_unsigned_false",
            .bytecode = le_unsigned_false_bytecode,
            .bytecode_size = sizeof(le_unsigned_false_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ge_unsigned_true",
            .bytecode = ge_unsigned_true_bytecode,
            .bytecode_size = sizeof(ge_unsigned_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ge_unsigned_false",
            .bytecode = ge_unsigned_false_bytecode,
            .bytecode_size = sizeof(ge_unsigned_false_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {0},  // false
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "eq_signed_true",
            .bytecode = eq_signed_true_bytecode,
            .bytecode_size = sizeof(eq_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ne_signed_true",
            .bytecode = ne_signed_true_bytecode,
            .bytecode_size = sizeof(ne_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "lt_signed_true",
            .bytecode = lt_signed_true_bytecode,
            .bytecode_size = sizeof(lt_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "gt_signed_true",
            .bytecode = gt_signed_true_bytecode,
            .bytecode_size = sizeof(gt_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "le_signed_true",
            .bytecode = le_signed_true_bytecode,
            .bytecode_size = sizeof(le_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ge_signed_true",
            .bytecode = ge_signed_true_bytecode,
            .bytecode_size = sizeof(ge_signed_true_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1},  // true
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "unsigned_vs_signed",
            .bytecode = unsigned_vs_signed_bytecode,
            .bytecode_size = sizeof(unsigned_vs_signed_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {1, 0},  // Stack: [false (unsigned), true (signed)]
            .expected_stack_size = 2,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "comparison_stack_underflow",
            .bytecode = comparison_stack_underflow_bytecode,
            .bytecode_size = sizeof(comparison_stack_underflow_bytecode),
            .expected_error = VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        }
};

// GT Lite test suite structure
const gt_lite_test_suite_t comparisons_test_suite = {
    .suite_name = "comparisons",
    .test_count = 12,
    .tests = comparisons_tests
};