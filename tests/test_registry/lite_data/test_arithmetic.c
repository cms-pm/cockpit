#include "../test_runner/include/gt_lite_test_types.h"
#include <assert.h>

// GT Lite Arithmetic Operations Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: Basic ADD operation - PUSH(15), PUSH(25), ADD, HALT
static const uint8_t add_basic_bytecode[] = {
    0x01, 0x00, 0x0F, 0x00,  // PUSH(15) - opcode=0x01, flags=0x00, immediate=15
    0x01, 0x00, 0x19, 0x00,  // PUSH(25) - opcode=0x01, flags=0x00, immediate=25
    0x03, 0x00, 0x00, 0x00,  // ADD - opcode=0x03
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 2: Basic SUB operation - PUSH(50), PUSH(20), SUB, HALT
static const uint8_t sub_basic_bytecode[] = {
    0x01, 0x00, 0x32, 0x00,  // PUSH(50) - opcode=0x01, flags=0x00, immediate=50
    0x01, 0x00, 0x14, 0x00,  // PUSH(20) - opcode=0x01, flags=0x00, immediate=20
    0x04, 0x00, 0x00, 0x00,  // SUB - opcode=0x04
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 3: Basic MUL operation - PUSH(6), PUSH(7), MUL, HALT
static const uint8_t mul_basic_bytecode[] = {
    0x01, 0x00, 0x06, 0x00,  // PUSH(6) - opcode=0x01, flags=0x00, immediate=6
    0x01, 0x00, 0x07, 0x00,  // PUSH(7) - opcode=0x01, flags=0x00, immediate=7
    0x05, 0x00, 0x00, 0x00,  // MUL - opcode=0x05
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 4: Basic DIV operation - PUSH(84), PUSH(4), DIV, HALT
static const uint8_t div_basic_bytecode[] = {
    0x01, 0x00, 0x54, 0x00,  // PUSH(84) - opcode=0x01, flags=0x00, immediate=84
    0x01, 0x00, 0x04, 0x00,  // PUSH(4) - opcode=0x01, flags=0x00, immediate=4
    0x06, 0x00, 0x00, 0x00,  // DIV - opcode=0x06
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 5: Division by zero error - PUSH(42), PUSH(0), DIV
static const uint8_t div_by_zero_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, flags=0x00, immediate=42
    0x01, 0x00, 0x00, 0x00,  // PUSH(0) - opcode=0x01, flags=0x00, immediate=0
    0x06, 0x00, 0x00, 0x00,  // DIV - opcode=0x06 (division by zero)
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00 (never reached)
};

// Test 6: Stack underflow on arithmetic - POP with empty stack, then ADD
static const uint8_t arithmetic_stack_underflow_bytecode[] = {
    0x01, 0x00, 0x10, 0x00,  // PUSH(16) - only one operand
    0x03, 0x00, 0x00, 0x00,  // ADD - opcode=0x03 (requires 2 operands, underflow)
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00 (never reached)
};

// Compile-time size validation
static_assert(sizeof(add_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "add_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(sub_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "sub_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(mul_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "mul_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(div_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "div_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(div_by_zero_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "div_by_zero_bytecode exceeds 100-element limit");
static_assert(sizeof(arithmetic_stack_underflow_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "arithmetic_stack_underflow_bytecode exceeds 100-element limit");

// GT Lite test suite definition
const gt_lite_test_suite_t arithmetic_test_suite = {
    .suite_name = "arithmetic_operations",
    .test_count = 6,
    .tests = (const gt_lite_test_t[]){
        {
            .test_name = "add_basic",
            .bytecode = add_basic_bytecode,
            .bytecode_size = sizeof(add_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {40},  // 15 + 25 = 40
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "sub_basic",
            .bytecode = sub_basic_bytecode,
            .bytecode_size = sizeof(sub_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {30},  // 50 - 20 = 30
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "mul_basic",
            .bytecode = mul_basic_bytecode,
            .bytecode_size = sizeof(mul_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {42},  // 6 * 7 = 42
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "div_basic",
            .bytecode = div_basic_bytecode,
            .bytecode_size = sizeof(div_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {21},  // 84 / 4 = 21
            .expected_stack_size = 1,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "div_by_zero",
            .bytecode = div_by_zero_bytecode,
            .bytecode_size = sizeof(div_by_zero_bytecode),
            .expected_error = VM_ERROR_DIVISION_BY_ZERO,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "arithmetic_stack_underflow",
            .bytecode = arithmetic_stack_underflow_bytecode,
            .bytecode_size = sizeof(arithmetic_stack_underflow_bytecode),
            .expected_error = VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        }
    }
};