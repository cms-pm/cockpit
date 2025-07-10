/*
 * Arduino Function Tests - Migrated to ComponentVM
 * Phase 3: Updated to use ComponentVM C wrapper interface
 */

#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"

// Test result tracking
typedef struct {
    int passed;
    int failed;
    int total;
} test_results_t;

static volatile test_results_t arduino_results = {0, 0, 0};

// Enhanced test assertion macro with semihosting output
#define GPIO_TEST_ASSERT(condition, name) do { \
    arduino_results.total++; \
    semihost_write_string("Test: "); \
    semihost_write_string(name); \
    semihost_write_string(" ... "); \
    if (condition) { \
        arduino_results.passed++; \
        semihost_write_string("PASS\n"); \
    } else { \
        arduino_results.failed++; \
        semihost_write_string("FAIL\n"); \
    } \
} while(0)

// Test pinMode() VM opcode
void test_pin_mode_opcode_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    // Test program: Set pin 13 to OUTPUT mode
    vm_instruction_c_t pin_mode_program[] = {
        {0x01, 0, 1},           // OP_PUSH 1 (OUTPUT)
        {0x17, 0, 13},          // OP_PIN_MODE (pin 13)
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, pin_mode_program, 3);
    GPIO_TEST_ASSERT(result, "Pin mode program execution");
    
    component_vm_destroy(vm);
}

// Test millis() function
void test_millis_function_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    // Test program: Get millis() value
    vm_instruction_c_t millis_program[] = {
        {0x19, 0, 0},           // OP_MILLIS
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, millis_program, 2);
    GPIO_TEST_ASSERT(result, "Millis program execution");
    GPIO_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "No errors");

    component_vm_destroy(vm);
}

// Main test runner for Arduino integration tests
int run_arduino_function_tests(void) {
    debug_print("\n=== Phase 2: Arduino Integration Tests (Migrated) ===\n");

    // Reset results
    arduino_results.passed = 0;
    arduino_results.failed = 0;
    arduino_results.total = 0;

    // Run all migrated tests
    test_pin_mode_opcode_migrated();
    test_millis_function_migrated();

    // Print summary
    semihost_write_string("\n--- Arduino Function Test Summary ---\n");
    semihost_write_string("Passed: ");
    semihost_write_dec(arduino_results.passed);
    semihost_write_string("\nFailed: ");
    semihost_write_dec(arduino_results.failed);
    semihost_write_string("\nTotal:  ");
    semihost_write_dec(arduino_results.total);
    semihost_write_string("\n");

    if (arduino_results.failed == 0) {
        debug_print("✓ Phase 2 functionality validated");
    }

    return arduino_results.failed;
}
