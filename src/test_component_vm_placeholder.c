/*
 * ComponentVM Tests using C Wrapper
 * Phase 3: C-compatible tests for ComponentVM using wrapper interface
 */

#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"

// Test result tracking
typedef struct {
    int passed;
    int failed;
    int total;
} test_results_t;

static volatile test_results_t results = {0, 0, 0};

// Enhanced test assertion macro with semihosting output
#define TEST_ASSERT(condition, name) do { \
    results.total++; \
    semihost_write_string("Test: "); \
    semihost_write_string(name); \
    semihost_write_string(" ... "); \
    if (condition) { \
        results.passed++; \
        semihost_write_string("PASS\n"); \
    } else { \
        results.failed++; \
        semihost_write_string("FAIL\n"); \
    } \
} while(0)

// Test ComponentVM wrapper initialization
void test_component_vm_wrapper_init(void) {
    ComponentVM_C* vm = component_vm_create();
    
    TEST_ASSERT(vm != NULL, "VM wrapper creation");
    TEST_ASSERT(!component_vm_is_running(vm), "VM not running initially");
    TEST_ASSERT(!component_vm_is_halted(vm), "VM not halted initially");
    TEST_ASSERT(component_vm_get_last_error(vm) == VM_C_ERROR_NONE, "No initial errors");
    TEST_ASSERT(component_vm_get_instruction_count(vm) == 0, "Zero instruction count");
    
    component_vm_destroy(vm);
}

// Test program loading through wrapper
void test_component_vm_wrapper_program_loading(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test with null program
    bool result = component_vm_load_program(vm, NULL, 0);
    TEST_ASSERT(!result, "Null program rejected");
    TEST_ASSERT(component_vm_get_last_error(vm) == VM_C_ERROR_PROGRAM_NOT_LOADED, "Correct error for null program");
    
    // Test with valid program (simple HALT)
    vm_instruction_c_t halt_program[] = {{0x00, 0, 0}};  // OP_HALT
    result = component_vm_load_program(vm, halt_program, 1);
    TEST_ASSERT(result, "Valid program loaded");
    TEST_ASSERT(component_vm_get_last_error(vm) == VM_C_ERROR_NONE, "No error after valid load");
    
    component_vm_destroy(vm);
}

// Test simple program execution through wrapper
void test_component_vm_wrapper_execution(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Simple program: PUSH 42, HALT
    vm_instruction_c_t simple_program[] = {
        {0x01, 0, 42},  // OP_PUSH 42
        {0x00, 0, 0}    // OP_HALT
    };
    
    bool result = component_vm_execute_program(vm, simple_program, 2);
    TEST_ASSERT(result, "Simple program executed");
    TEST_ASSERT(component_vm_is_halted(vm), "VM halted after execution");
    TEST_ASSERT(component_vm_get_last_error(vm) == VM_C_ERROR_NONE, "No execution errors");
    
    component_vm_destroy(vm);
}

// Test VM reset functionality through wrapper
void test_component_vm_wrapper_reset(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Load and execute a simple program
    vm_instruction_c_t simple_program[] = {
        {0x01, 0, 100}, // OP_PUSH 100
        {0x00, 0, 0}    // OP_HALT
    };
    
    component_vm_execute_program(vm, simple_program, 2);
    TEST_ASSERT(component_vm_is_halted(vm), "VM halted after execution");
    
    // Reset the VM
    component_vm_reset(vm);
    TEST_ASSERT(!component_vm_is_running(vm), "VM not running after reset");
    TEST_ASSERT(!component_vm_is_halted(vm), "VM not halted after reset");
    TEST_ASSERT(component_vm_get_instruction_count(vm) == 0, "Instruction count reset");
    TEST_ASSERT(component_vm_get_last_error(vm) == VM_C_ERROR_NONE, "No errors after reset");
    
    component_vm_destroy(vm);
}

// Test error handling through wrapper
void test_component_vm_wrapper_error_handling(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test error string function
    const char* error_str = component_vm_get_error_string(VM_C_ERROR_NONE);
    TEST_ASSERT(error_str != NULL, "Error string function works");
    
    error_str = component_vm_get_error_string(VM_C_ERROR_STACK_OVERFLOW);
    TEST_ASSERT(error_str != NULL, "Stack overflow error string");
    
    component_vm_destroy(vm);
}

// Test legacy compatibility functions
void test_component_vm_legacy_compatibility(void) {
    ComponentVM_C* vm = NULL;
    
    // Test legacy init
    int result = vm_init_compat(&vm);
    TEST_ASSERT(result == 0, "Legacy init compatibility");
    TEST_ASSERT(vm != NULL, "Legacy init creates VM");
    
    // Test legacy program format conversion (16-bit to 32-bit)
    uint16_t legacy_program[] = {0x0000}; // HALT in old format
    result = vm_load_program_compat(vm, legacy_program, 1);
    TEST_ASSERT(result == 0, "Legacy program loading");
    
    // Test legacy execution
    result = vm_run_compat(vm, 1000); // max_cycles ignored
    TEST_ASSERT(result == 0, "Legacy execution compatibility");
    
    component_vm_destroy(vm);
}

// Run all ComponentVM wrapper tests
int run_component_vm_tests(void) {
    semihost_write_string("\n=== ComponentVM C Wrapper Tests ===\n");
    
    results.passed = 0;
    results.failed = 0;
    results.total = 0;
    
    test_component_vm_wrapper_init();
    test_component_vm_wrapper_program_loading();
    test_component_vm_wrapper_execution();
    test_component_vm_wrapper_reset();
    test_component_vm_wrapper_error_handling();
    test_component_vm_legacy_compatibility();
    
    // Print summary
    semihost_write_string("\n--- ComponentVM Test Summary ---\n");
    semihost_write_string("Passed: ");
    semihost_write_dec(results.passed);
    semihost_write_string("\nFailed: ");
    semihost_write_dec(results.failed);
    semihost_write_string("\nTotal:  ");
    semihost_write_dec(results.total);
    semihost_write_string("\n");
    
    if (results.failed == 0) {
        semihost_write_string("✓ ComponentVM C wrapper working correctly\n");
        semihost_write_string("✓ Legacy vm_core migration successful\n");
        semihost_write_string("✓ Mixed C/C++ compilation ready\n");
    }
    
    return results.failed;
}