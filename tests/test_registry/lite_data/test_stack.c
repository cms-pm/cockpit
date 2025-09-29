#include "../test_runner/include/gt_lite_test_types.h"
#include <assert.h>

// GT Lite Stack Operations Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: Basic PUSH operation - PUSH(42), HALT
static const uint8_t push_basic_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, flags=0x00, immediate=42
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 2: Basic POP operation - PUSH(99), POP, HALT
static const uint8_t pop_basic_bytecode[] = {
    0x01, 0x00, 0x63, 0x00,  // PUSH(99) - opcode=0x01, flags=0x00, immediate=99
    0x02, 0x00, 0x00, 0x00,  // POP - opcode=0x02
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 3: Stack underflow error - POP on empty stack
static const uint8_t stack_underflow_bytecode[] = {
    0x02, 0x00, 0x00, 0x00,  // POP - opcode=0x02 (on empty stack)
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00 (never reached)
};

// Test 4: Multiple PUSH operations
static const uint8_t multi_push_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH(10)
    0x01, 0x00, 0x14, 0x00,  // PUSH(20)
    0x01, 0x00, 0x1E, 0x00,  // PUSH(30)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Compile-time size validation
static_assert(sizeof(push_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "push_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(pop_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "pop_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(stack_underflow_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "stack_underflow_bytecode exceeds 100-element limit");
static_assert(sizeof(multi_push_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "multi_push_bytecode exceeds 100-element limit");

// GT Lite test suite definition
const gt_lite_test_suite_t stack_test_suite = {
    .suite_name = "stack_operations",
    .test_count = 4,
    .tests = (const gt_lite_test_t[]){
        {
            .test_name = "push_basic",
            .bytecode = push_basic_bytecode,
            .bytecode_size = sizeof(push_basic_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {42},
            .expected_stack_size = 1,
            .memory_address = 0,      // No memory validation for basic test
            .expected_memory_value = 0
        },
        {
            .test_name = "pop_basic",
            .bytecode = pop_basic_bytecode,
            .bytecode_size = sizeof(pop_basic_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {},     // Empty stack after POP
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "stack_underflow",
            .bytecode = stack_underflow_bytecode,
            .bytecode_size = sizeof(stack_underflow_bytecode),
            .expected_error = GT_LITE_VM_ERROR_STACK_UNDERFLOW,
            .expected_stack = {},
            .expected_stack_size = 0,
            .memory_address = 0,
            .expected_memory_value = 0
        },
        {
            .test_name = "multi_push",
            .bytecode = multi_push_bytecode,
            .bytecode_size = sizeof(multi_push_bytecode),
            .expected_error = GT_LITE_VM_ERROR_NONE,
            .expected_stack = {10, 20, 30},  // Stack grows upward in memory
            .expected_stack_size = 3,
            .memory_address = 0,
            .expected_memory_value = 0
        }
    }
};