#include "../test_runner/include/gt_lite_test_types.h"
#include <assert.h>

// GT Lite Comparison Operations Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: Equal comparison (true) - PUSH(42), PUSH(42), EQ, HALT
static const uint8_t eq_true_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, flags=0x00, immediate=42
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, flags=0x00, immediate=42
    0x20, 0x00, 0x00, 0x00,  // EQ - opcode=0x20
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 2: Equal comparison (false) - PUSH(42), PUSH(43), EQ, HALT
static const uint8_t eq_false_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, flags=0x00, immediate=42
    0x01, 0x00, 0x2B, 0x00,  // PUSH(43) - opcode=0x01, flags=0x00, immediate=43
    0x20, 0x00, 0x00, 0x00,  // EQ - opcode=0x20
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 3: Not equal comparison (true) - PUSH(10), PUSH(20), NE, HALT
static const uint8_t ne_true_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH(10) - opcode=0x01, flags=0x00, immediate=10
    0x01, 0x00, 0x14, 0x00,  // PUSH(20) - opcode=0x01, flags=0x00, immediate=20
    0x21, 0x00, 0x00, 0x00,  // NE - opcode=0x21
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 4: Not equal comparison (false) - PUSH(15), PUSH(15), NE, HALT
static const uint8_t ne_false_bytecode[] = {
    0x01, 0x00, 0x0F, 0x00,  // PUSH(15) - opcode=0x01, flags=0x00, immediate=15
    0x01, 0x00, 0x0F, 0x00,  // PUSH(15) - opcode=0x01, flags=0x00, immediate=15
    0x21, 0x00, 0x00, 0x00,  // NE - opcode=0x21
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 5: Less than comparison (true) - PUSH(5), PUSH(10), LT, HALT
static const uint8_t lt_true_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH(5) - opcode=0x01, flags=0x00, immediate=5
    0x01, 0x00, 0x0A, 0x00,  // PUSH(10) - opcode=0x01, flags=0x00, immediate=10
    0x22, 0x00, 0x00, 0x00,  // LT - opcode=0x22
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 6: Less than comparison (false) - PUSH(20), PUSH(10), LT, HALT
static const uint8_t lt_false_bytecode[] = {
    0x01, 0x00, 0x14, 0x00,  // PUSH(20) - opcode=0x01, flags=0x00, immediate=20
    0x01, 0x00, 0x0A, 0x00,  // PUSH(10) - opcode=0x01, flags=0x00, immediate=10
    0x22, 0x00, 0x00, 0x00,  // LT - opcode=0x22
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 7: Greater than comparison (true) - PUSH(25), PUSH(15), GT, HALT
static const uint8_t gt_true_bytecode[] = {
    0x01, 0x00, 0x19, 0x00,  // PUSH(25) - opcode=0x01, flags=0x00, immediate=25
    0x01, 0x00, 0x0F, 0x00,  // PUSH(15) - opcode=0x01, flags=0x00, immediate=15
    0x23, 0x00, 0x00, 0x00,  // GT - opcode=0x23
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 8: Greater than comparison (false) - PUSH(8), PUSH(12), GT, HALT
static const uint8_t gt_false_bytecode[] = {
    0x01, 0x00, 0x08, 0x00,  // PUSH(8) - opcode=0x01, flags=0x00, immediate=8
    0x01, 0x00, 0x0C, 0x00,  // PUSH(12) - opcode=0x01, flags=0x00, immediate=12
    0x23, 0x00, 0x00, 0x00,  // GT - opcode=0x23
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 9: Stack underflow on comparison - Only one operand for EQ
static const uint8_t comparison_stack_underflow_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - only one operand
    0x20, 0x00, 0x00, 0x00,  // EQ - opcode=0x20 (requires 2 operands, underflow)
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00 (never reached)
};

// Compile-time size validation
static_assert(sizeof(eq_true_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "eq_true_bytecode exceeds 100-element limit");
static_assert(sizeof(eq_false_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "eq_false_bytecode exceeds 100-element limit");
static_assert(sizeof(ne_true_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "ne_true_bytecode exceeds 100-element limit");
static_assert(sizeof(ne_false_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "ne_false_bytecode exceeds 100-element limit");
static_assert(sizeof(lt_true_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "lt_true_bytecode exceeds 100-element limit");
static_assert(sizeof(lt_false_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "lt_false_bytecode exceeds 100-element limit");
static_assert(sizeof(gt_true_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "gt_true_bytecode exceeds 100-element limit");
static_assert(sizeof(gt_false_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "gt_false_bytecode exceeds 100-element limit");
static_assert(sizeof(comparison_stack_underflow_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "comparison_stack_underflow_bytecode exceeds 100-element limit");

// GT Lite test suite definition
const gt_lite_test_suite_t comparison_test_suite = {
    .suite_name = "comparison_operations",
    .test_count = 9,
    .tests = (const gt_lite_test_t[]){
        {
            .test_name = "eq_true",
            .bytecode = eq_true_bytecode,
            .bytecode_size = sizeof(eq_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true = 1
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "eq_false",
            .bytecode = eq_false_bytecode,
            .bytecode_size = sizeof(eq_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false = 0
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ne_true",
            .bytecode = ne_true_bytecode,
            .bytecode_size = sizeof(ne_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true = 1
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "ne_false",
            .bytecode = ne_false_bytecode,
            .bytecode_size = sizeof(ne_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false = 0
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "lt_true",
            .bytecode = lt_true_bytecode,
            .bytecode_size = sizeof(lt_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true = 1 (5 < 10)
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "lt_false",
            .bytecode = lt_false_bytecode,
            .bytecode_size = sizeof(lt_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false = 0 (20 not < 10)
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "gt_true",
            .bytecode = gt_true_bytecode,
            .bytecode_size = sizeof(gt_true_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {1},  // true = 1 (25 > 15)
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "gt_false",
            .bytecode = gt_false_bytecode,
            .bytecode_size = sizeof(gt_false_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {0},  // false = 0 (8 not > 12)
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "comparison_stack_underflow",
            .bytecode = comparison_stack_underflow_bytecode,
            .bytecode_size = sizeof(comparison_stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        }
    }
};