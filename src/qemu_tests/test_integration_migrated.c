/*
 * Phase 3 Integration Tests - Migrated to ComponentVM
 * Phase 3: Updated to use ComponentVM C wrapper interface
 * Tests complex scenarios combining multiple VM features
 */

#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"

// Test result tracking
typedef struct {
    int passed;
    int failed;
    int total;
} test_results_t;

static volatile test_results_t integration_results = {0, 0, 0};

// Enhanced test assertion macro with semihosting output
#define INTEGRATION_TEST_ASSERT(condition, name) do { \
    integration_results.total++; \
    semihost_write_string("Test: "); \
    semihost_write_string(name); \
    semihost_write_string(" ... "); \
    if (condition) { \
        integration_results.passed++; \
        semihost_write_string("PASS\n"); \
    } else { \
        integration_results.failed++; \
        semihost_write_string("FAIL\n"); \
    } \
} while(0)

// Test basic SOS pattern (simplified version)
void test_sos_pattern_basic_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    // Simple SOS pattern test (one complete sequence)
    // SOS = 3 short, 3 long, 3 short
    vm_instruction_c_t simple_sos[] = {
        // Setup LED pin
        {0x01, 0, 1},           // OP_PUSH 1 (OUTPUT mode)
        {0x17, 0, 13},          // OP_PIN_MODE 13 (LED output)
        
        // 3 Short signals (dots)
        {0x01, 0, 1},           // OP_PUSH 1 (HIGH)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED on)
        {0x01, 0, 100},         // OP_PUSH 100 (short delay)
        {0x14, 0, 0},           // OP_DELAY
        
        {0x01, 0, 0},           // OP_PUSH 0 (LOW)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED off)
        {0x01, 0, 100},         // OP_PUSH 100 (short pause)
        {0x14, 0, 0},           // OP_DELAY
        
        {0x01, 0, 1},           // OP_PUSH 1 (HIGH)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED on)
        {0x01, 0, 100},         // OP_PUSH 100 (short delay)
        {0x14, 0, 0},           // OP_DELAY
        
        {0x01, 0, 0},           // OP_PUSH 0 (LOW)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED off)
        {0x01, 0, 100},         // OP_PUSH 100 (short pause)
        {0x14, 0, 0},           // OP_DELAY
        
        {0x01, 0, 1},           // OP_PUSH 1 (HIGH)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED on)
        {0x01, 0, 100},         // OP_PUSH 100 (short delay)
        {0x14, 0, 0},           // OP_DELAY
        
        {0x01, 0, 0},           // OP_PUSH 0 (LOW)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED off)
        
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, simple_sos, sizeof(simple_sos)/sizeof(simple_sos[0]));
    INTEGRATION_TEST_ASSERT(result, "SOS pattern basic execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "SOS pattern completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "No errors during SOS");

    component_vm_destroy(vm);
}

// Test C-to-bytecode Level 1: Basic Digital Output
// C Code equivalent: pinMode(13, OUTPUT); digitalWrite(13, HIGH);
void test_c2b_level1_basic_output_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    vm_instruction_c_t basic_output[] = {
        {0x01, 0, 1},           // OP_PUSH 1 (OUTPUT mode)
        {0x17, 0, 13},          // OP_PIN_MODE 13 (pinMode(13, OUTPUT))
        {0x01, 0, 1},           // OP_PUSH 1 (HIGH state)
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (digitalWrite(13, HIGH))
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, basic_output, 5);
    INTEGRATION_TEST_ASSERT(result, "C2B Level 1.1: Basic output execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "C2B Level 1.1: Program completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "C2B Level 1.1: No errors");

    component_vm_destroy(vm);
}

// Test C-to-bytecode Level 1: Analog Input Reading
// C Code equivalent: analogRead(0);
void test_c2b_level1_analog_input_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    vm_instruction_c_t analog_input[] = {
        {0x13, 0, 0},           // OP_ANALOG_READ pin 0 (result on stack)
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, analog_input, 2);
    INTEGRATION_TEST_ASSERT(result, "C2B Level 1.2: Analog input execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "C2B Level 1.2: Program completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "C2B Level 1.2: No errors");

    component_vm_destroy(vm);
}

// Test C-to-bytecode Level 1: Timing Functions
// C Code equivalent: delay(100); unsigned long time = millis();
void test_c2b_level1_timing_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    vm_instruction_c_t timing_example[] = {
        {0x01, 0, 100},         // OP_PUSH 100 (delay time)
        {0x14, 0, 0},           // OP_DELAY (delay(100ms))
        {0x19, 0, 0},           // OP_MILLIS (time = millis(), result on stack)
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, timing_example, 4);
    INTEGRATION_TEST_ASSERT(result, "C2B Level 1.3: Timing execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "C2B Level 1.3: Program completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "C2B Level 1.3: No errors");

    component_vm_destroy(vm);
}

// Test arithmetic operations integration
void test_arithmetic_integration_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    // Complex arithmetic: (10 + 20) * 3 - 5 = 85
    vm_instruction_c_t arithmetic_program[] = {
        {0x01, 0, 10},          // OP_PUSH 10
        {0x01, 0, 20},          // OP_PUSH 20
        {0x03, 0, 0},           // OP_ADD (30 on stack)
        {0x01, 0, 3},           // OP_PUSH 3
        {0x05, 0, 0},           // OP_MUL (90 on stack)
        {0x01, 0, 5},           // OP_PUSH 5
        {0x04, 0, 0},           // OP_SUB (85 on stack)
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, arithmetic_program, 8);
    INTEGRATION_TEST_ASSERT(result, "Arithmetic integration execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "Arithmetic integration completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "Arithmetic integration no errors");

    component_vm_destroy(vm);
}

// Test comprehensive Arduino HAL integration
void test_arduino_hal_integration_migrated(void) {
    ComponentVM_C* vm = component_vm_create();

    // Multi-function Arduino program: setup pins, read input, control output
    vm_instruction_c_t hal_integration[] = {
        // Setup pins
        {0x01, 0, 1},           // OP_PUSH 1 (OUTPUT mode)
        {0x17, 0, 13},          // OP_PIN_MODE 13 (LED output)
        {0x01, 0, 0},           // OP_PUSH 0 (INPUT mode)
        {0x17, 0, 2},           // OP_PIN_MODE 2 (button input)
        
        // Read digital input
        {0x11, 0, 2},           // OP_DIGITAL_READ pin 2 (result on stack)
        
        // Control digital output based on input
        {0x10, 0, 13},          // OP_DIGITAL_WRITE 13 (LED = button state)
        
        // Get timing information
        {0x19, 0, 0},           // OP_MILLIS (get current time)
        
        {0x00, 0, 0}            // OP_HALT
    };

    bool result = component_vm_execute_program(vm, hal_integration, 8);
    INTEGRATION_TEST_ASSERT(result, "Arduino HAL integration execution");
    INTEGRATION_TEST_ASSERT(component_vm_is_halted(vm), "Arduino HAL integration completed");
    INTEGRATION_TEST_ASSERT(component_vm_get_last_error(vm) == VM_ERROR_NONE, "Arduino HAL integration no errors");

    component_vm_destroy(vm);
}

// Main test runner for Phase 3 integration tests
int run_integration_tests(void) {
    debug_print("\n=== Phase 3: Integration Tests (Migrated) ===");

    // Reset results
    integration_results.passed = 0;
    integration_results.failed = 0;
    integration_results.total = 0;

    // Run all migrated integration tests
    test_sos_pattern_basic_migrated();
    test_c2b_level1_basic_output_migrated();
    test_c2b_level1_analog_input_migrated();
    test_c2b_level1_timing_migrated();
    test_arithmetic_integration_migrated();
    test_arduino_hal_integration_migrated();

    // Print summary
    semihost_write_string("\n--- Integration Test Summary ---\n");
    semihost_write_string("Passed: ");
    semihost_write_dec(integration_results.passed);
    semihost_write_string("\nFailed: ");
    semihost_write_dec(integration_results.failed);
    semihost_write_string("\nTotal:  ");
    semihost_write_dec(integration_results.total);
    semihost_write_string("\n");

    if (integration_results.failed == 0) {
        debug_print("✓ Phase 3 integration tests successful");
        debug_print("✓ SOS demo functionality validated");
        debug_print("✓ C-to-bytecode patterns working");
        debug_print("✓ Complex scenarios operational");
    }

    return integration_results.failed;
}
