/*
 * C-to-Bytecode Examples and Integration Tests - Phase 2.3.4
 * 
 * This file implements executable examples that demonstrate C code patterns
 * and their corresponding VM bytecode implementations, serving as validation
 * for Phase 3 C compiler development.
 */

#include "test_gpio_common.h"
#include "../lib/vm_core/vm_core.h"
#include "../lib/button_input/button_input.h"

// Test results
static gpio_test_results_t c2b_results = {0, 0, 0};

// ============================================================================
// LEVEL 1 EXAMPLES: Single Arduino Functions
// ============================================================================

// Example 1.1: Basic Digital Output
// C Code: pinMode(13, OUTPUT); digitalWrite(13, HIGH);
void test_c2b_level1_basic_output(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: None (constants only)
    uint16_t basic_output[] = {
        (OP_PUSH << 8) | 1,          // Push OUTPUT mode (1)
        (OP_PIN_MODE << 8) | 13,     // pinMode(13, OUTPUT)
        (OP_PUSH << 8) | 1,          // Push HIGH state (1)
        (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(13, HIGH)
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, basic_output, 5);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.1: Program load", &c2b_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.1: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 10, "Level 1.1: Performance (<10 cycles)", &c2b_results);
    
    debug_print_dec("Level 1.1 cycles", execution_cycles);
}

// Example 1.2: Analog Input Reading  
// C Code: analogRead(0);
void test_c2b_level1_analog_input(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: None (result on stack)
    uint16_t analog_input[] = {
        (OP_ANALOG_READ << 8) | 0,   // Read analog pin A0, result on stack
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, analog_input, 2);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.2: Program load", &c2b_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.2: Execution", &c2b_results);
    
    // Verify result is on stack
    uint32_t result;
    error = vm_pop(&vm, &result);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.2: Result available", &c2b_results);
    GPIO_TEST_ASSERT(result >= 0 && result <= 1023, "Level 1.2: Valid ADC range", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 10, "Level 1.2: Performance (<10 cycles)", &c2b_results);
    
    debug_print_dec("Level 1.2 cycles", execution_cycles);
}

// Example 1.3: Timing Function
// C Code: delay(100); unsigned long time = millis();
void test_c2b_level1_timing(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: time@stack[-1] (millis result)
    uint16_t timing_example[] = {
        (OP_DELAY << 8) | 100,       // delay(100ms)
        (OP_MILLIS << 8) | 0,        // time = millis() (result on stack[-1])
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, timing_example, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.3: Program load", &c2b_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.3: Execution", &c2b_results);
    
    // Verify timing result
    uint32_t time_result;
    error = vm_pop(&vm, &time_result);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 1.3: Time result available", &c2b_results);
    GPIO_TEST_ASSERT(time_result >= 100, "Level 1.3: Time advanced by delay", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 10, "Level 1.3: Performance (<10 cycles)", &c2b_results);
    
    debug_print_dec("Level 1.3 cycles", execution_cycles);
}

// ============================================================================
// LEVEL 2 EXAMPLES: Multiple Functions + Variables
// ============================================================================

// Example 2.1: LED Control with Variable
// C Code: int pin = 13; pinMode(pin, OUTPUT); digitalWrite(pin, HIGH); delay(500); digitalWrite(pin, LOW);
void test_c2b_level2_led_control_var(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: pin@stack[-1] (LED pin number)
    uint16_t led_control_var[] = {
        (OP_PUSH << 8) | 13,         // int pin = 13 (pin@stack[-1])
        (OP_PUSH << 8) | 1,          // Push OUTPUT mode
        (OP_PIN_MODE << 8) | 13,     // pinMode(pin, OUTPUT) - use immediate for now
        (OP_PUSH << 8) | 1,          // Push HIGH state
        (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(pin, HIGH)
        (OP_DELAY << 8) | 244,       // delay(500) - use closest 8-bit value (244)
        (OP_PUSH << 8) | 0,          // Push LOW state  
        (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(pin, LOW)
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, led_control_var, 9);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.1: Program load", &c2b_results);
    
    error = vm_run(&vm, 200);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.1: Execution", &c2b_results);
    
    // Verify pin variable is still on stack
    uint32_t pin_value;
    error = vm_pop(&vm, &pin_value);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.1: Pin variable available", &c2b_results);
    GPIO_TEST_ASSERT(pin_value == 13, "Level 2.1: Pin variable correct", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 25, "Level 2.1: Performance (<25 cycles)", &c2b_results);
    
    debug_print_dec("Level 2.1 cycles", execution_cycles);
}

// Example 2.2: Sensor Reading with Calculation
// C Code: int raw = analogRead(0); int scaled = raw / 4; analogWrite(9, scaled);
void test_c2b_level2_sensor_calculation(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: raw@stack[-1], scaled@stack[-2] after calculation
    uint16_t sensor_calculation[] = {
        (OP_ANALOG_READ << 8) | 0,   // raw = analogRead(0) (raw@stack[-1])
        (OP_PUSH << 8) | 4,          // Push divisor 4
        (OP_DIV << 8) | 0,           // scaled = raw / 4 (scaled@stack[-1], raw consumed)
        (OP_ANALOG_WRITE << 8) | 9,  // analogWrite(9, scaled) - pops scaled from stack
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, sensor_calculation, 5);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.2: Program load", &c2b_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.2: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 25, "Level 2.2: Performance (<25 cycles)", &c2b_results);
    
    debug_print_dec("Level 2.2 cycles", execution_cycles);
}

// Example 2.3: Multiple Arduino Functions Integration
// C Code: pinMode(13, OUTPUT); pinMode(2, INPUT); int button = digitalRead(2); 
//         digitalWrite(13, button); printf("Button: %d\n", button);
void test_c2b_level2_multi_function(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: button@stack[-1] (button state 0 or 1)
    uint16_t multi_function[] = {
        (OP_PUSH << 8) | 1,          // Push OUTPUT mode
        (OP_PIN_MODE << 8) | 13,     // pinMode(13, OUTPUT)
        (OP_PUSH << 8) | 0,          // Push INPUT mode
        (OP_PIN_MODE << 8) | 2,      // pinMode(2, INPUT)
        (OP_DIGITAL_READ << 8) | 2,  // button = digitalRead(2) (button@stack[-1])
        
        // Duplicate button value for both digitalWrite and printf
        (OP_PUSH << 8) | 0,          // Push 0 for duplication
        (OP_ADD << 8) | 0,           // button + 0 = button (duplicates value on stack)
        (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(13, button) - pops one copy
        
        (OP_PUSH << 8) | 1,          // Push arg count for printf
        (OP_PRINTF << 8) | 1,        // printf("Value: %d", button) - uses remaining copy
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, multi_function, 11);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.3: Program load", &c2b_results);
    
    error = vm_run(&vm, 200);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 2.3: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 25, "Level 2.3: Performance (<25 cycles)", &c2b_results);
    
    debug_print_dec("Level 2.3 cycles", execution_cycles);
}

// ============================================================================
// LEVEL 3 EXAMPLES: Conditionals + Complex Logic (Simplified for Current VM)
// ============================================================================

// Example 3.1: Sensor Threshold with Conditional (Current VM Implementation)
// C Code: int sensor = analogRead(0); int threshold = 512; if (sensor > threshold) { ... } else { ... }
void test_c2b_level3_sensor_threshold(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: sensor@stack[-1], threshold@stack[-2], comparison_result@stack[-1]
    uint16_t sensor_threshold_current[] = {
        (OP_ANALOG_READ << 8) | 0,       // sensor = analogRead(0)
        (OP_PUSH << 8) | 200,            // threshold = 200 (simplified for 8-bit)
        (OP_GT << 8) | 0,                // sensor > threshold (comparison result on stack)
        
        // Simulate conditional execution (Phase 3 will have actual JMP)
        (OP_PUSH << 8) | 1,              // Push HIGH for demonstration
        (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, HIGH) - always executes for demo
        
        // Printf with comparison result to show conditional logic
        (OP_PUSH << 8) | 1,              // Push arg count
        (OP_PRINTF << 8) | 1,            // printf("Value: %d", comparison_result)
        
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, sensor_threshold_current, 8);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.1: Program load", &c2b_results);
    
    error = vm_run(&vm, 200);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.1: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 50, "Level 3.1: Performance (<50 cycles)", &c2b_results);
    
    debug_print_dec("Level 3.1 cycles", execution_cycles);
}

// Example 3.2: Arithmetic Operations and Comparisons
// C Code: int a = 10; int b = 20; int sum = a + b; int diff = b - a; 
//         if (sum > 25) { printf("Sum large: %d\n", sum); }
void test_c2b_level3_arithmetic_comparison(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Variable Table: a@stack[-3], b@stack[-2], sum@stack[-1], diff@stack[varies]
    uint16_t arithmetic_comparison[] = {
        (OP_PUSH << 8) | 10,         // int a = 10
        (OP_PUSH << 8) | 20,         // int b = 20
        (OP_ADD << 8) | 0,           // sum = a + b (pops a,b pushes sum)
        
        (OP_PUSH << 8) | 20,         // Push b again (for subtraction)
        (OP_PUSH << 8) | 10,         // Push a again (for subtraction)
        (OP_SUB << 8) | 0,           // diff = b - a (pops a,b pushes diff)
        
        // Compare sum > 25
        (OP_PUSH << 8) | 25,         // Push comparison value
        (OP_GT << 8) | 0,            // sum > 25 (comparison result on stack)
        
        // Print comparison result
        (OP_PUSH << 8) | 1,          // Push arg count
        (OP_PRINTF << 8) | 1,        // printf("Value: %d", comparison_result)
        
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, arithmetic_comparison, 11);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.2: Program load", &c2b_results);
    
    error = vm_run(&vm, 200);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.2: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 50, "Level 3.2: Performance (<50 cycles)", &c2b_results);
    
    debug_print_dec("Level 3.2 cycles", execution_cycles);
}

// Example 3.3: Complex Arduino Integration (Simplified)
// C Code: Complex timing + sensor + conditional logic pattern
void test_c2b_level3_complex_integration(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Simplified version of complex example from documentation
    uint16_t complex_integration[] = {
        (OP_MILLIS << 8) | 0,            // start = millis()
        (OP_ANALOG_READ << 8) | 0,       // sensor = analogRead(0)
        (OP_ANALOG_READ << 8) | 1,       // sensor2 = analogRead(1)
        (OP_ADD << 8) | 0,               // sensor_sum = sensor + sensor2
        
        (OP_PUSH << 8) | 2,              // Push divisor
        (OP_DIV << 8) | 0,               // average = sensor_sum / 2
        
        (OP_MILLIS << 8) | 0,            // current_time = millis()
        (OP_SUB << 8) | 0,               // elapsed = current_time - start
        
        // Print results
        (OP_PUSH << 8) | 1,              // Push arg count
        (OP_PRINTF << 8) | 1,            // printf("Value: %d", average)
        
        (OP_HALT << 8) | 0
    };
    
    uint32_t start_cycles = vm.cycle_count;
    vm_error_t error = vm_load_program(&vm, complex_integration, 10);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.3: Program load", &c2b_results);
    
    error = vm_run(&vm, 200);
    GPIO_TEST_ASSERT(error == VM_OK, "Level 3.3: Execution", &c2b_results);
    
    uint32_t execution_cycles = vm.cycle_count - start_cycles;
    GPIO_TEST_ASSERT(execution_cycles < 50, "Level 3.3: Performance (<50 cycles)", &c2b_results);
    
    debug_print_dec("Level 3.3 cycles", execution_cycles);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

// Main C-to-bytecode test runner
int run_c_to_bytecode_tests(void) {
    reset_gpio_test_results(&c2b_results);
    
    debug_print("=== C-to-Bytecode Examples Tests Starting ===");
    
    // Level 1: Single Arduino Functions
    debug_print("--- Level 1: Single Arduino Functions ---");
    test_c2b_level1_basic_output();
    test_c2b_level1_analog_input();
    test_c2b_level1_timing();
    
    // Level 2: Multiple Functions + Variables
    debug_print("--- Level 2: Multiple Functions + Variables ---");
    test_c2b_level2_led_control_var();
    test_c2b_level2_sensor_calculation();
    test_c2b_level2_multi_function();
    
    // Level 3: Conditionals + Complex Logic (Simplified)
    debug_print("--- Level 3: Conditionals + Complex Logic ---");
    test_c2b_level3_sensor_threshold();
    test_c2b_level3_arithmetic_comparison();
    test_c2b_level3_complex_integration();
    
    print_gpio_test_summary("C-to-Bytecode Examples", &c2b_results);
    
    return c2b_results.failed;
}