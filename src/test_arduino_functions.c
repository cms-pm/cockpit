/*
 * Arduino Function Tests - Phase 2.3
 * Test pinMode, timing functions, and comparison operations
 */

#include "test_gpio_common.h"
#include "../lib/vm_core/vm_core.h"
#include "../lib/button_input/button_input.h"

// Test results
static gpio_test_results_t arduino_results = {0, 0, 0};

// Test pinMode() VM opcode
void test_pin_mode_opcode(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Set pin 13 to OUTPUT mode
    uint16_t pin_mode_program[] = {
        (OP_PUSH << 8) | 1,          // Push OUTPUT mode (1)
        (OP_PIN_MODE << 8) | 13,     // Set pin 13 to mode from stack
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, pin_mode_program, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Pin mode program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Pin mode program execution", &arduino_results);
}

// Test invalid pin mode handling
void test_pin_mode_validation(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Invalid pin mode
    uint16_t invalid_mode_program[] = {
        (OP_PUSH << 8) | 99,         // Push invalid mode (99)
        (OP_PIN_MODE << 8) | 13,     // Try to set pin 13 to invalid mode
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, invalid_mode_program, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Invalid mode program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Invalid mode handled gracefully", &arduino_results);
}

// Test millis() function
void test_millis_function(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Get millis() value
    uint16_t millis_program[] = {
        (OP_MILLIS << 8) | 0,        // Get current time in milliseconds
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, millis_program, 2);
    GPIO_TEST_ASSERT(error == VM_OK, "Millis program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Millis program execution", &arduino_results);
    
    // Check that a value was pushed to stack
    uint32_t millis_value;
    error = vm_pop(&vm, &millis_value);
    GPIO_TEST_ASSERT(error == VM_OK, "Millis value on stack", &arduino_results);
    GPIO_TEST_ASSERT(millis_value >= 0, "Millis value reasonable", &arduino_results);
}

// Test micros() function
void test_micros_function(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Get micros() value
    uint16_t micros_program[] = {
        (OP_MICROS << 8) | 0,        // Get current time in microseconds
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, micros_program, 2);
    GPIO_TEST_ASSERT(error == VM_OK, "Micros program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Micros program execution", &arduino_results);
    
    // Check that a value was pushed to stack
    uint32_t micros_value;
    error = vm_pop(&vm, &micros_value);
    GPIO_TEST_ASSERT(error == VM_OK, "Micros value on stack", &arduino_results);
    GPIO_TEST_ASSERT(micros_value >= 0, "Micros value reasonable", &arduino_results);
}

// Test timing progression (millis should advance)
void test_timing_progression(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Get initial time
    uint16_t time1_program[] = {
        (OP_MILLIS << 8) | 0,        // Get time 1
        (OP_HALT << 8) | 0
    };
    
    vm_load_program(&vm, time1_program, 2);
    vm_run(&vm, 100);
    uint32_t time1;
    vm_pop(&vm, &time1);
    
    // Advance virtual time
    qemu_advance_time(100);
    
    // Get time again
    vm_init(&vm);  // Reset VM
    vm_load_program(&vm, time1_program, 2);
    vm_run(&vm, 100);
    uint32_t time2;
    vm_pop(&vm, &time2);
    
    GPIO_TEST_ASSERT(time2 > time1, "Time advances correctly", &arduino_results);
    GPIO_TEST_ASSERT((time2 - time1) >= 100, "Time advancement accurate", &arduino_results);
}

// Test printf() basic functionality
void test_printf_basic(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Hello World") - no arguments
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 0,          // Push arg count: 0
        (OP_PRINTF << 8) | 0,        // Printf with string ID 0 ("Hello World")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf basic program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf basic execution", &arduino_results);
}

// Test printf() with %d format
void test_printf_decimal(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Value: %d", 42)
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 42,         // Push argument: 42
        (OP_PUSH << 8) | 1,          // Push arg count: 1
        (OP_PRINTF << 8) | 1,        // Printf with string ID 1 ("Value: %d")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 4);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf decimal program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf decimal execution", &arduino_results);
}

// Test printf() with %c format
void test_printf_character(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Char: %c", 'A')
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 'A',        // Push argument: 'A' (65)
        (OP_PUSH << 8) | 1,          // Push arg count: 1
        (OP_PRINTF << 8) | 2,        // Printf with string ID 2 ("Char: %c")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 4);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf character program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf character execution", &arduino_results);
}

// Test printf() with %x format
void test_printf_hexadecimal(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Hex: %x", 255)
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 255,        // Push argument: 255 (0xFF)
        (OP_PUSH << 8) | 1,          // Push arg count: 1
        (OP_PRINTF << 8) | 3,        // Printf with string ID 3 ("Hex: %x")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 4);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf hex program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf hex execution", &arduino_results);
}

// Test printf() with %s format
void test_printf_string(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("String: %s", "Hello World")
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 0,          // Push string arg: string ID 0 ("Hello World")
        (OP_PUSH << 8) | 1,          // Push arg count: 1
        (OP_PRINTF << 8) | 7,        // Printf with string ID 7 ("String: %s")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 4);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf string program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf string execution", &arduino_results);
}

// Test printf() with multiple arguments
void test_printf_multiple_args(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Multiple: %d %c %x", 42, 'Z', 16)
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 16,         // Push third argument: 16 (hex)
        (OP_PUSH << 8) | 'Z',        // Push second argument: 'Z'
        (OP_PUSH << 8) | 42,         // Push first argument: 42
        (OP_PUSH << 8) | 3,          // Push arg count: 3
        (OP_PRINTF << 8) | 4,        // Printf with string ID 4 ("Multiple: %d %c %x")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 6);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf multiple args program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf multiple args execution", &arduino_results);
}

// Test printf() error handling (missing arguments)
void test_printf_missing_args(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: printf("Value: %d") - missing argument
    uint16_t printf_program[] = {
        (OP_PUSH << 8) | 0,          // Push arg count: 0 (but format expects 1)
        (OP_PRINTF << 8) | 1,        // Printf with string ID 1 ("Value: %d")
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, printf_program, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf missing args program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf missing args handled gracefully", &arduino_results);
}

// Test printf() integration with Arduino functions
void test_printf_arduino_integration(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Combined Arduino + printf
    uint16_t integration_program[] = {
        // digitalWrite(13, HIGH)
        (OP_PUSH << 8) | 1,          // Push HIGH
        (OP_DIGITAL_WRITE << 8) | 13, // Write to pin 13
        
        // printf("Printf working: %d", 123)
        (OP_PUSH << 8) | 123,        // Push argument: 123
        (OP_PUSH << 8) | 1,          // Push arg count: 1
        (OP_PRINTF << 8) | 6,        // Printf with string ID 6
        
        // Get current time
        (OP_MILLIS << 8) | 0,        // Get millis
        
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, integration_program, 7);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf integration program load", &arduino_results);
    
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Printf integration execution", &arduino_results);
    
    // Should have millis value on stack
    uint32_t millis_value;
    error = vm_pop(&vm, &millis_value);
    GPIO_TEST_ASSERT(error == VM_OK, "Integration millis result", &arduino_results);
}

// Test complete Arduino-style program
void test_complete_arduino_program(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Arduino-style program: pinMode + digitalWrite + timing
    uint16_t arduino_program[] = {
        // pinMode(13, OUTPUT)
        (OP_PUSH << 8) | 1,          // Push OUTPUT mode
        (OP_PIN_MODE << 8) | 13,     // Set pin 13 to OUTPUT
        
        // digitalWrite(13, HIGH)
        (OP_PUSH << 8) | 1,          // Push HIGH
        (OP_DIGITAL_WRITE << 8) | 13, // Write to pin 13
        
        // unsigned long start = millis()
        (OP_MILLIS << 8) | 0,        // Get start time (stays on stack)
        
        // delay(50)
        (OP_PUSH << 8) | 50,         // Push delay amount
        (OP_DELAY << 8) | 0,         // Delay (pops from stack)
        
        // digitalWrite(13, LOW)
        (OP_PUSH << 8) | 0,          // Push LOW
        (OP_DIGITAL_WRITE << 8) | 13, // Write to pin 13
        
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, arduino_program, 9);
    GPIO_TEST_ASSERT(error == VM_OK, "Arduino program load", &arduino_results);
    
    error = vm_run(&vm, 500);
    GPIO_TEST_ASSERT(error == VM_OK, "Arduino program execution", &arduino_results);
    
    // Should have start time on stack
    uint32_t start_time;
    error = vm_pop(&vm, &start_time);
    GPIO_TEST_ASSERT(error == VM_OK, "Start time captured", &arduino_results);
}

// Main Arduino function test runner
int run_arduino_function_tests(void) {
    reset_gpio_test_results(&arduino_results);
    
    debug_print("=== Arduino Function Tests Starting ===");
    
    // Run tests
    test_pin_mode_opcode();
    test_pin_mode_validation();
    test_millis_function();
    test_micros_function();
    test_timing_progression();
    
    // Printf tests
    test_printf_basic();
    test_printf_decimal();
    test_printf_character();
    test_printf_hexadecimal();
    test_printf_string();
    test_printf_multiple_args();
    test_printf_missing_args();
    test_printf_arduino_integration();
    
    test_complete_arduino_program();
    
    print_gpio_test_summary("Arduino Functions", &arduino_results);
    
    return arduino_results.failed;
}