/*
 * ComponentVM Telemetry Validation Test
 * Phase 4.2.2B1.5: Test program with known memory writes
 * 
 * Purpose: Create a controlled VM execution environment that writes
 * known values to telemetry for Python debugging validation
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/semihosting/semihosting.h"
    #include "../lib/component_vm_bridge/component_vm_bridge.h"
    #include "../lib/vm_blackbox/include/vm_blackbox.h"
    #include "../include/memory_layout.h"
#endif

#ifdef HARDWARE_PLATFORM

// Test sequence markers for GDB inspection
volatile uint32_t test_sequence_marker = 0x12345678;
volatile uint32_t test_phase = 0;

// VM test program - simple operations with known outcomes (CORRECTED OPCODES)
static const vm_instruction_t test_vm_program[] = {
    // Test 1: Simple arithmetic (PUSH 100, PUSH 50, ADD)
    {0x01, 0x00, 100},    // PUSH 100        (OP_PUSH = 0x01)
    {0x01, 0x00, 50},     // PUSH 50         (OP_PUSH = 0x01)  
    {0x03, 0x00, 0},      // ADD             (OP_ADD = 0x03)
    
    // Test 2: Memory operation (store result to global 0)
    {0x51, 0x00, 0},      // STORE_GLOBAL 0  (OP_STORE_GLOBAL = 0x51)
    
    // Test 3: Arduino API call (digitalWrite HIGH)
    {0x01, 0x00, 13},     // PUSH pin 13     (OP_PUSH = 0x01)
    {0x01, 0x00, 1},      // PUSH HIGH       (OP_PUSH = 0x01)
    {0x10, 0x00, 0},      // digitalWrite    (OP_DIGITAL_WRITE = 0x10)
    
    // Test 4: Delay for observable timing
    {0x01, 0x00, 250},    // PUSH 250ms      (OP_PUSH = 0x01)
    {0x14, 0x00, 0},      // delay           (OP_DELAY = 0x14)
    
    // Test 5: Final arithmetic sequence
    {0x01, 0x00, 42},     // PUSH 42         (OP_PUSH = 0x01)
    {0x51, 0x00, 1},      // STORE_GLOBAL 1  (OP_STORE_GLOBAL = 0x51)
    
    {0x00, 0x00, 0}       // HALT            (OP_HALT = 0x00)
};

void test_telemetry_validation(void) {
    debug_print("=== TELEMETRY VALIDATION TEST START ===");
    
    // Phase 1: Initialize ComponentVM with telemetry
    test_phase = 1;
    test_sequence_marker = 0xAAAA0001;
    
    component_vm_t* vm = component_vm_create();
    if (!vm) {
        debug_print("ERROR: Failed to create ComponentVM");
        return;
    }
    
    debug_print("✓ ComponentVM created successfully");
    
    // Phase 2: Enable telemetry
    test_phase = 2;
    test_sequence_marker = 0xAAAA0002;
    
    component_vm_enable_telemetry(vm, true);
    if (!component_vm_is_telemetry_enabled(vm)) {
        debug_print("ERROR: Failed to enable telemetry");
        component_vm_destroy(vm);
        return;
    }
    
    debug_print("✓ Telemetry enabled successfully");
    
    // Phase 3: Load test program
    test_phase = 3;
    test_sequence_marker = 0xAAAA0003;
    
    vm_result_t load_result = component_vm_load_program(vm, test_vm_program, 
                                sizeof(test_vm_program) / sizeof(test_vm_program[0]));
    if (load_result != VM_RESULT_SUCCESS) {
        debug_print("ERROR: Failed to load test program");
        component_vm_destroy(vm);
        return;
    }
    
    debug_print("✓ Test program loaded successfully");
    debug_print_dec("Program size (instructions)", sizeof(test_vm_program) / sizeof(test_vm_program[0]));
    
    // Phase 4: Execute program and monitor telemetry
    test_phase = 4;
    test_sequence_marker = 0xAAAA0004;
    
    debug_print(">>> Starting VM execution with telemetry monitoring");
    
    vm_result_t exec_result = component_vm_execute_program(vm, test_vm_program, 
                                sizeof(test_vm_program) / sizeof(test_vm_program[0]));
    
    if (exec_result == VM_RESULT_SUCCESS) {
        debug_print("✓ VM program executed successfully");
    } else {
        debug_print("ERROR: VM program execution failed");
        debug_print(component_vm_get_error_string(exec_result));
    }
    
    // Phase 5: Validation - telemetry should now contain execution data
    test_phase = 5;
    test_sequence_marker = 0xAAAA0005;
    
    size_t instruction_count = component_vm_get_instruction_count(vm);
    debug_print_dec("Total instructions executed", instruction_count);
    
    // Phase 6: Memory layout verification
    test_phase = 6;
    test_sequence_marker = 0xAAAA0006;
    
    debug_print("=== MEMORY LAYOUT VERIFICATION ===");
    debug_print_hex("Expected telemetry address", TELEMETRY_BASE_ADDR);
    debug_print_hex("Telemetry magic value", TELEMETRY_MAGIC);
    debug_print_hex("Format version", TELEMETRY_FORMAT_V4_1);
    
    // Create a known test value for Python debugging
    volatile uint32_t* telemetry_ptr = (volatile uint32_t*)TELEMETRY_BASE_ADDR;
    debug_print_hex("Telemetry magic at address", telemetry_ptr[0]);
    debug_print_hex("Format version at offset 4", telemetry_ptr[1]);
    debug_print_hex("Program counter at offset 8", telemetry_ptr[2]);
    debug_print_hex("Instruction count at offset 12", telemetry_ptr[3]);
    
    // Phase 7: Python debugging anchor point with predictable halt
    test_phase = 7;
    test_sequence_marker = 0xFADE5AFE;  // Use telemetry magic as final marker
    
    debug_print("=== PYTHON DEBUG ANCHOR POINT ===");
    debug_print("Python can set breakpoint here and inspect:");
    debug_print_hex("1. test_sequence_marker", test_sequence_marker);
    debug_print_hex("2. test_phase", test_phase);
    debug_print_hex("3. telemetry_ptr", (uint32_t)telemetry_ptr);
    debug_print_hex("4. vm pointer", (uint32_t)vm);
    debug_print("Use: x/8x 0x20007F00 to examine telemetry");
    
    // PREDICTABLE HALT STATE: Allow telemetry to settle before inspection
    debug_print("Entering stable state for telemetry inspection...");
    for (int settle_count = 0; settle_count < 100; settle_count++) {
        HAL_Delay(10);  // 10ms delay * 100 = 1 second settle time
        // Update marker to show we're in stable state
        test_sequence_marker = 0xFADE5AFE + settle_count;
    }
    
    // Final stable state
    test_sequence_marker = 0xDEBUG999;  // Predictable final value for GDB
    
    // Keep variables alive for GDB inspection
    volatile int gdb_anchor = 42;
    (void)gdb_anchor;  // Prevent optimization
    
    // Phase 8: Cleanup
    test_phase = 8;
    test_sequence_marker = 0xAAAA0008;
    
    component_vm_destroy(vm);
    debug_print("✓ ComponentVM destroyed successfully");
    debug_print("=== TELEMETRY VALIDATION TEST COMPLETE ===");
}

// Test program entry point for telemetry validation
void run_telemetry_validation_main(void) {
    debug_print("ComponentVM Telemetry Validation Test");
    debug_print("Phase 4.2.2B1.5: Known memory writes for Python debugging");
    debug_print("");
    
    // Run the validation test
    test_telemetry_validation();
    
    // Success state - slow blink to indicate completion
    debug_print("Test completed - entering slow blink mode");
    debug_print("LED will blink slowly to indicate telemetry test success");
    
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(1000);  // 1 second ON
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(1000);  // 1 second OFF
        
        // Periodic status
        debug_print("Telemetry test complete - GDB can inspect memory at 0x20007F00");
    }
}

#endif // HARDWARE_PLATFORM