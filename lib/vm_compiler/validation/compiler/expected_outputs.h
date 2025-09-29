/*
 * Expected Outputs for Runtime Validation Tests
 * Defines test specifications for bytecode execution validation
 */

#pragma once

typedef struct {
    const char* test_name;
    const char* expected_output;
    const char* expected_error;
    int should_fail;
    int use_pattern_matching;
} runtime_test_spec_t;

// Test specifications for runtime validation
static const runtime_test_spec_t runtime_test_specs[] = {
    // Basic arithmetic tests
    {"test_minimal", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_32bit", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_boolean", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_global_only", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_gt_only", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_printf_only", "EXECUTION_SUCCESS", NULL, 0, 1},
    
    // Negative number test
    {"test_negative", "EXECUTION_SUCCESS", NULL, 0, 1},
    
    // Priority 1: Core operations opcode tests
    {"test_arithmetic_ops", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_stack_ops", "EXECUTION_SUCCESS", NULL, 0, 1},
    
    // Priority 2: Essential comparison & logical operations
    {"test_comparisons", "EXECUTION_SUCCESS", NULL, 0, 1},
    {"test_logical_simple", "EXECUTION_SUCCESS", NULL, 0, 1},
    
    // End marker
    {NULL, NULL, NULL, 0, 0}
};