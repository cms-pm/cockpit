/*
 * ComponentVM Hardware Integration Test
 * Phase 4.2.1C: Bytecode Test Programs for STM32G431CB
 * 
 * Hardcoded bytecode program executing GPIO operations via ComponentVM
 */

#ifdef HARDWARE_PLATFORM

#include "../lib/component_vm_bridge/component_vm_bridge.h"
#include "../lib/arduino_hal/arduino_hal.h"
#include "../lib/semihosting/semihosting.h"

// Arduino API instruction opcodes (from ComponentVM instruction set)
#define OP_HALT           0x00
#define OP_PUSH_CONST     0x01
#define OP_ARDUINO_PINMODE      0x40
#define OP_ARDUINO_DIGITALWRITE 0x41
#define OP_ARDUINO_DELAY        0x42

// Pin mode constants
#define PIN_MODE_OUTPUT   1

// Pin state constants  
#define PIN_HIGH          1
#define PIN_LOW           0

// LED pin (Arduino pin 13 = PC6)
#define LED_PIN           13

// Hardcoded bytecode program: LED blink pattern
// This program demonstrates VM executing Arduino API calls
static const vm_instruction_t led_blink_program[] = {
    // Initialize LED pin as output
    {OP_PUSH_CONST, 0, LED_PIN},           // Push pin number (13)
    {OP_PUSH_CONST, 0, PIN_MODE_OUTPUT},   // Push pin mode (OUTPUT)
    {OP_ARDUINO_PINMODE, 0, 0},            // Call pinMode(13, OUTPUT)
    
    // Main blink loop (5 cycles)
    // Cycle 1: LED ON
    {OP_PUSH_CONST, 0, LED_PIN},           // Push pin number
    {OP_PUSH_CONST, 0, PIN_HIGH},          // Push HIGH state
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       // Call digitalWrite(13, HIGH)
    {OP_PUSH_CONST, 0, 500},               // Push delay time (500ms)
    {OP_ARDUINO_DELAY, 0, 0},              // Call delay(500)
    
    // Cycle 1: LED OFF
    {OP_PUSH_CONST, 0, LED_PIN},           // Push pin number
    {OP_PUSH_CONST, 0, PIN_LOW},           // Push LOW state
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       // Call digitalWrite(13, LOW)
    {OP_PUSH_CONST, 0, 500},               // Push delay time (500ms)
    {OP_ARDUINO_DELAY, 0, 0},              // Call delay(500)
    
    // Cycle 2: LED ON
    {OP_PUSH_CONST, 0, LED_PIN},           
    {OP_PUSH_CONST, 0, PIN_HIGH},          
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       
    {OP_PUSH_CONST, 0, 500},               
    {OP_ARDUINO_DELAY, 0, 0},              
    
    // Cycle 2: LED OFF
    {OP_PUSH_CONST, 0, LED_PIN},           
    {OP_PUSH_CONST, 0, PIN_LOW},           
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       
    {OP_PUSH_CONST, 0, 500},               
    {OP_ARDUINO_DELAY, 0, 0},              
    
    // Cycle 3: LED ON
    {OP_PUSH_CONST, 0, LED_PIN},           
    {OP_PUSH_CONST, 0, PIN_HIGH},          
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       
    {OP_PUSH_CONST, 0, 500},               
    {OP_ARDUINO_DELAY, 0, 0},              
    
    // Cycle 3: LED OFF
    {OP_PUSH_CONST, 0, LED_PIN},           
    {OP_PUSH_CONST, 0, PIN_LOW},           
    {OP_ARDUINO_DIGITALWRITE, 0, 0},       
    {OP_PUSH_CONST, 0, 500},               
    {OP_ARDUINO_DELAY, 0, 0},              
    
    // Program completion
    {OP_HALT, 0, 0}                        // Halt execution
};

#define PROGRAM_SIZE (sizeof(led_blink_program) / sizeof(vm_instruction_t))

void test_vm_hardware_integration(void) {
    debug_print("=== ComponentVM Hardware Integration Test ===");
    debug_print_dec("Program size (instructions)", PROGRAM_SIZE);
    
    // Initialize Arduino HAL
    hal_gpio_init();
    debug_print("Arduino HAL initialized");
    
    // Create ComponentVM instance
    component_vm_t* vm = component_vm_create();
    if (!vm) {
        debug_print("ERROR: Failed to create ComponentVM instance");
        return;
    }
    
    debug_print("ComponentVM instance created successfully");
    
    // Load the hardcoded bytecode program
    vm_result_t load_result = component_vm_load_program(vm, led_blink_program, PROGRAM_SIZE);
    if (load_result != VM_RESULT_SUCCESS) {
        debug_print("ERROR: Failed to load bytecode program");
        debug_print(component_vm_get_error_string(load_result));
        component_vm_destroy(vm);
        return;
    }
    
    debug_print("Bytecode program loaded successfully");
    
    // Execute the program
    debug_print("Starting bytecode execution...");
    vm_result_t exec_result = component_vm_execute_program(vm, led_blink_program, PROGRAM_SIZE);
    
    if (exec_result == VM_RESULT_SUCCESS) {
        debug_print("✓ Bytecode execution completed successfully");
        
        // Get performance metrics
        vm_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
        debug_print("=== Performance Metrics ===");
        debug_print_dec("Execution time (ms)", metrics.execution_time_ms);
        debug_print_dec("Instructions executed", metrics.instructions_executed);
        debug_print_dec("Memory operations", metrics.memory_operations);
        debug_print_dec("I/O operations", metrics.io_operations);
        
        // Verify expected instruction count
        size_t instruction_count = component_vm_get_instruction_count(vm);
        debug_print_dec("Total instruction count", instruction_count);
        
        if (instruction_count == PROGRAM_SIZE) {
            debug_print("✓ Instruction count matches program size");
        } else {
            debug_print("⚠ Instruction count mismatch");
        }
        
    } else {
        debug_print("✗ Bytecode execution failed");
        debug_print(component_vm_get_error_string(exec_result));
    }
    
    // Verify VM state
    if (component_vm_is_halted(vm)) {
        debug_print("✓ VM properly halted after execution");
    } else {
        debug_print("⚠ VM still running after execution");
    }
    
    // Cleanup
    component_vm_destroy(vm);
    debug_print("ComponentVM instance destroyed");
    
    debug_print("=== Hardware Integration Test Complete ===");
}

#endif // HARDWARE_PLATFORM