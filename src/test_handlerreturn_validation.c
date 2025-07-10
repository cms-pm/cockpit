/*
 * HandlerReturn Architecture Validation Tests
 * Phase 3.9: Validates new explicit PC management and stack protection
 */

#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"
#include <stdlib.h>

// Test result tracking
typedef struct {
    int passed;
    int failed;
    int total;
} handlerreturn_test_results_t;

static volatile handlerreturn_test_results_t hr_results = {0, 0, 0};

// Enhanced test assertion macro
#define HR_TEST_ASSERT(condition, name) do { \
    hr_results.total++; \
    if (condition) { \
        hr_results.passed++; \
        debug_print("Test: " name " ... PASS"); \
    } else { \
        hr_results.failed++; \
        debug_print("Test: " name " ... FAIL"); \
    } \
} while(0)

// Test 1: Nested Function Calls (3 levels)
void test_nested_calls_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Program: main() -> func1() -> func2() -> return chain
    vm_instruction_c_t nested_program[] = {
        // main function (address 0-2)
        {0x08, 0, 3},   // CALL func1 (address 3)
        {0x00, 0, 0},   // HALT
        {0x00, 0, 0},   // padding
        
        // func1 (address 3-5)
        {0x08, 0, 6},   // CALL func2 (address 6)
        {0x09, 0, 0},   // RET
        {0x00, 0, 0},   // padding
        
        // func2 (address 6-8)
        {0x01, 0, 42},  // PUSH 42
        {0x02, 0, 0},   // POP (discard)
        {0x09, 0, 0},   // RET
    };
    
    bool result = component_vm_execute_program(vm, nested_program, 9);
    HR_TEST_ASSERT(result, "Nested function calls execution");
    HR_TEST_ASSERT(component_vm_is_halted(vm), "Nested calls halted correctly");
    HR_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "No errors in nested calls");
    
    component_vm_destroy(vm);
}

// Test 2: Error Boundary Validation
void test_error_boundaries_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test invalid return address
    vm_instruction_c_t invalid_ret_program[] = {
        {0x01, 0, 255}, // PUSH 255 (invalid address for small program)
        {0x09, 0, 0},   // RET (should fail with bounds check)
        {0x00, 0, 0},   // HALT (should not reach)
    };
    
    bool result = component_vm_execute_program(vm, invalid_ret_program, 3);
    HR_TEST_ASSERT(!result, "Invalid return address detected");
    
    component_vm_reset(vm);
    
    // Test invalid call address  
    vm_instruction_c_t invalid_call_program[] = {
        {0x08, 0, 255}, // CALL 255 (invalid address)
        {0x00, 0, 0},   // HALT (should not reach)
    };
    
    result = component_vm_execute_program(vm, invalid_call_program, 2);
    HR_TEST_ASSERT(!result, "Invalid call address detected");
    
    component_vm_destroy(vm);
}

// Test 3: Stack Underflow Protection
void test_stack_underflow_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test stack underflow (RET without CALL)
    vm_instruction_c_t underflow_program[] = {
        {0x09, 0, 0},   // RET (no corresponding CALL)
        {0x00, 0, 0},   // HALT (should not reach)
    };
    
    bool result = component_vm_execute_program(vm, underflow_program, 2);
    HR_TEST_ASSERT(!result, "Stack underflow on RET detected");
    
    component_vm_destroy(vm);
}

// Test 4: CALL/RET Balance Validation
void test_call_ret_balance_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test balanced CALL/RET
    vm_instruction_c_t balanced_program[] = {
        {0x08, 0, 2},   // CALL function (address 2)
        {0x00, 0, 0},   // HALT
        {0x09, 0, 0},   // RET (balanced - no stack modification)
    };
    
    bool result = component_vm_execute_program(vm, balanced_program, 3);
    
    // Debug output to understand the failure
    if (!result) {
        debug_print("CALL/RET execution failed - debugging:");
        debug_print_dec("Error code", (int)component_vm_get_last_error(vm));
        debug_print_dec("Is halted", component_vm_is_halted(vm) ? 1 : 0);
        #ifdef DEBUG
        debug_print("Build mode: DEBUG");
        #else
        debug_print("Build mode: RELEASE");
        #endif
    }
    
    HR_TEST_ASSERT(result, "Balanced CALL/RET execution");
    HR_TEST_ASSERT(component_vm_is_halted(vm), "Balanced CALL/RET halted correctly");
    HR_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "No errors in balanced CALL/RET");
    
    component_vm_destroy(vm);
}

// Test 5: Deep Nesting Stress Test
void test_deep_nesting_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test 5-level deep nesting (conservative for embedded)
    vm_instruction_c_t deep_program[] = {
        // main -> func1 -> func2 -> func3 -> func4 -> func5
        {0x08, 0, 2},   // CALL func1 (address 2)
        {0x00, 0, 0},   // HALT
        {0x08, 0, 4},   // func1: CALL func2 (address 4)
        {0x09, 0, 0},   // RET
        {0x08, 0, 6},   // func2: CALL func3 (address 6)
        {0x09, 0, 0},   // RET
        {0x08, 0, 8},   // func3: CALL func4 (address 8)
        {0x09, 0, 0},   // RET
        {0x08, 0, 10},  // func4: CALL func5 (address 10)
        {0x09, 0, 0},   // RET
        {0x01, 0, 200}, // func5: PUSH 200
        {0x02, 0, 0},   // POP
        {0x09, 0, 0},   // RET
    };
    
    bool result = component_vm_execute_program(vm, deep_program, 13);
    HR_TEST_ASSERT(result, "Deep nesting (5 levels) execution");
    HR_TEST_ASSERT(component_vm_is_halted(vm), "Deep nesting halted correctly");
    HR_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "No errors in deep nesting");
    
    component_vm_destroy(vm);
}

// Test 6: Jump Instruction Validation
void test_jump_validation_handlerreturn(void) {
    ComponentVM_C* vm = component_vm_create();
    
    // Test valid conditional jump
    vm_instruction_c_t jump_program[] = {
        {0x01, 0, 1},   // PUSH 1 (true condition)
        {0x31, 0, 4},   // JMP_TRUE to address 4
        {0x01, 0, 99},  // PUSH 99 (should skip)
        {0x00, 0, 0},   // HALT (should skip)
        {0x01, 0, 42},  // PUSH 42 (jump target)
        {0x00, 0, 0},   // HALT
    };
    
    bool result = component_vm_execute_program(vm, jump_program, 6);
    HR_TEST_ASSERT(result, "Valid conditional jump execution");
    HR_TEST_ASSERT(component_vm_is_halted(vm), "Jump program halted correctly");
    
    component_vm_reset(vm);
    
    // Test invalid jump address
    vm_instruction_c_t invalid_jump_program[] = {
        {0x01, 0, 1},   // PUSH 1 (true condition)
        {0x31, 0, 255}, // JMP_TRUE to invalid address 255
        {0x00, 0, 0},   // HALT (should not reach)
    };
    
    result = component_vm_execute_program(vm, invalid_jump_program, 3);
    HR_TEST_ASSERT(!result, "Invalid jump address detected");
    
    component_vm_destroy(vm);
}

int run_handlerreturn_validation_tests(void) {
    debug_print("\n=== HandlerReturn Architecture Validation Tests ===");
    
    // Reset test results
    hr_results.passed = 0;
    hr_results.failed = 0;
    hr_results.total = 0;
    
    // Run all tests
    test_nested_calls_handlerreturn();
    test_error_boundaries_handlerreturn();
    test_stack_underflow_handlerreturn();
    test_call_ret_balance_handlerreturn();
    test_deep_nesting_handlerreturn();
    test_jump_validation_handlerreturn();
    
    // Report results
    debug_print("\n--- HandlerReturn Validation Test Summary ---");
    debug_print_dec("Passed", hr_results.passed);
    debug_print_dec("Failed", hr_results.failed);
    debug_print_dec("Total", hr_results.total);
    
    if (hr_results.failed == 0) {
        debug_print("✓ All HandlerReturn architecture tests passed");
        debug_print("✓ Explicit PC management validated");
        debug_print("✓ Stack protection working correctly");
        debug_print("✓ Error boundary detection functional");
        debug_print("✓ Phase 3.9 HandlerReturn architecture ready");
    } else {
        debug_print("❌ Some HandlerReturn tests failed");
    }
    
    return hr_results.failed;
}