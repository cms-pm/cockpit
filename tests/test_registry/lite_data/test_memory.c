#include "../test_runner/include/gt_lite_test_types.h"
#include "vm_errors.h"

// Phase 4.13.4: Memory Operations GT Lite Test Data
// Testing global variables, local variables, and array operations

// Test 1: Global Variable Store and Load - PUSH 42 → STORE_GLOBAL 0 → LOAD_GLOBAL 0
static const uint8_t global_store_load_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH 42
    0x51, 0x00, 0x00, 0x00,  // STORE_GLOBAL 0
    0x50, 0x00, 0x00, 0x00,  // LOAD_GLOBAL 0
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 2: Multiple Globals - store 100 to global 1, 200 to global 2, load global 1
static const uint8_t multiple_globals_bytecode[] = {
    0x01, 0x00, 0x64, 0x00,  // PUSH 100
    0x51, 0x00, 0x01, 0x00,  // STORE_GLOBAL 1
    0x01, 0x00, 0xC8, 0x00,  // PUSH 200 (0xC8 = 200)
    0x51, 0x00, 0x02, 0x00,  // STORE_GLOBAL 2
    0x50, 0x00, 0x01, 0x00,  // LOAD_GLOBAL 1
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: Global Bounds Error - attempt to store to invalid global index 255
static const uint8_t global_bounds_error_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH 42
    0x51, 0x00, 0xFF, 0x00,  // STORE_GLOBAL 255 (invalid)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 4: Array Creation and Basic Access
static const uint8_t array_creation_bytecode[] = {
    0x01, 0x00, 0x0A, 0x00,  // PUSH 10 (array size)
    0x56, 0x00, 0x00, 0x00,  // CREATE_ARRAY 0
    0x01, 0x00, 0x2A, 0x00,  // PUSH 42 (value)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (index)
    0x55, 0x00, 0x00, 0x00,  // STORE_ARRAY 0
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (index)
    0x54, 0x00, 0x00, 0x00,  // LOAD_ARRAY 0
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: Array Multiple Elements
static const uint8_t array_multiple_elements_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (array size)
    0x56, 0x00, 0x00, 0x00,  // CREATE_ARRAY 0
    // Store 100 at index 0
    0x01, 0x00, 0x64, 0x00,  // PUSH 100
    0x01, 0x00, 0x00, 0x00,  // PUSH 0
    0x55, 0x00, 0x00, 0x00,  // STORE_ARRAY 0
    // Store 200 at index 1
    0x01, 0x00, 0xC8, 0x00,  // PUSH 200
    0x01, 0x00, 0x01, 0x00,  // PUSH 1
    0x55, 0x00, 0x00, 0x00,  // STORE_ARRAY 0
    // Load index 1
    0x01, 0x00, 0x01, 0x00,  // PUSH 1
    0x54, 0x00, 0x00, 0x00,  // LOAD_ARRAY 0
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 6: Array Bounds Error - create array size 3, access index 5
static const uint8_t array_bounds_error_bytecode[] = {
    0x01, 0x00, 0x03, 0x00,  // PUSH 3 (array size)
    0x56, 0x00, 0x00, 0x00,  // CREATE_ARRAY 0
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (invalid index)
    0x54, 0x00, 0x00, 0x00,  // LOAD_ARRAY 0 (should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 7: Array Invalid Size - attempt size 2000 (too large)
static const uint8_t array_invalid_size_bytecode[] = {
    0x01, 0x00, 0xD0, 0x07,  // PUSH 2000 (0x07D0 = 2000)
    0x56, 0x00, 0x00, 0x00,  // CREATE_ARRAY 0 (should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 8: Store Global Stack Underflow
static const uint8_t store_global_underflow_bytecode[] = {
    0x51, 0x00, 0x00, 0x00,  // STORE_GLOBAL 0 (no value on stack)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 9: Store Array Stack Underflow
static const uint8_t store_array_underflow_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (array size)
    0x56, 0x00, 0x00, 0x00,  // CREATE_ARRAY 0
    0x55, 0x00, 0x00, 0x00,  // STORE_ARRAY 0 (needs value and index - should fail)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 10: Complex Memory Operations - globals + arrays + arithmetic
static const uint8_t complex_memory_bytecode[] = {
    // Store 333 in global 0
    0x01, 0x00, 0x4D, 0x01,  // PUSH 333 (0x014D = 333)
    0x51, 0x00, 0x00, 0x00,  // STORE_GLOBAL 0
    // Create array and store 444 at index 1
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (array size)
    0x56, 0x00, 0x01, 0x00,  // CREATE_ARRAY 1
    0x01, 0x00, 0xBC, 0x01,  // PUSH 444 (0x01BC = 444)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1
    0x55, 0x00, 0x01, 0x00,  // STORE_ARRAY 1
    // Load global and array element, add them
    0x50, 0x00, 0x00, 0x00,  // LOAD_GLOBAL 0 (333)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1
    0x54, 0x00, 0x01, 0x00,  // LOAD_ARRAY 1 (444)
    0x03, 0x00, 0x00, 0x00,  // ADD (333 + 444 = 777)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// GT Lite test cases array
static const gt_lite_test_t memory_tests[] = {
    {
        .test_name = "global_store_load",
        .bytecode = global_store_load_bytecode,
        .bytecode_size = sizeof(global_store_load_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {42},
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 42
    },
    {
        .test_name = "multiple_globals",
        .bytecode = multiple_globals_bytecode,
        .bytecode_size = sizeof(multiple_globals_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {100},
        .expected_stack_size = 1,
        .memory_address = 1,
        .expected_memory_value = 100
    },
    {
        .test_name = "global_bounds_error",
        .bytecode = global_bounds_error_bytecode,
        .bytecode_size = sizeof(global_bounds_error_bytecode),
        .expected_error = VM_ERROR_MEMORY_BOUNDS,
        .expected_stack = {},
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "array_creation",
        .bytecode = array_creation_bytecode,
        .bytecode_size = sizeof(array_creation_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {42},
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "array_multiple_elements",
        .bytecode = array_multiple_elements_bytecode,
        .bytecode_size = sizeof(array_multiple_elements_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {200},
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "array_bounds_error",
        .bytecode = array_bounds_error_bytecode,
        .bytecode_size = sizeof(array_bounds_error_bytecode),
        .expected_error = VM_ERROR_MEMORY_BOUNDS,
        .expected_stack = {},
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "array_invalid_size",
        .bytecode = array_invalid_size_bytecode,
        .bytecode_size = sizeof(array_invalid_size_bytecode),
        .expected_error = VM_ERROR_MEMORY_BOUNDS,
        .expected_stack = {},
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "store_global_underflow",
        .bytecode = store_global_underflow_bytecode,
        .bytecode_size = sizeof(store_global_underflow_bytecode),
        .expected_error = VM_ERROR_STACK_UNDERFLOW,
        .expected_stack = {},
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "store_array_underflow",
        .bytecode = store_array_underflow_bytecode,
        .bytecode_size = sizeof(store_array_underflow_bytecode),
        .expected_error = VM_ERROR_STACK_UNDERFLOW,
        .expected_stack = {},
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "complex_memory",
        .bytecode = complex_memory_bytecode,
        .bytecode_size = sizeof(complex_memory_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {777},  // 333 + 444
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 333
    }
};

// GT Lite test suite structure
const gt_lite_test_suite_t memory_test_suite = {
    .suite_name = "memory",
    .test_count = 10,
    .tests = memory_tests
};