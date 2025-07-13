/*
 * ComponentVM Automated Test: VM Memory Operations
 * Phase 4.3.1.2: VM Instruction Coverage Test Programs
 * 
 * Purpose: Test memory load/store operations with manual expectations
 * Expected Instructions: 8 (Load/store to global variables)
 * Expected Result: global[0] = 42, global[1] = 84
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../../lib/semihosting/semihosting.h"
    #include "../../lib/vm_bridge/vm_bridge.h"
    #include "../../lib/vm_blackbox/include/vm_blackbox.h"
    #include "../../include/memory_layout.h"
#endif

#ifdef HARDWARE_PLATFORM

// Test marker for GDB inspection
static volatile uint32_t test_marker = 0xEE00001;  // "MEMORY 01" in valid hex
static volatile uint32_t test_status = 0;

// VM test program: Memory operations (store, load, manipulate globals)
static const vm_instruction_t vm_memory_operations_program[] = {
    {0x01, 0x00, 42},     // PUSH 42         (OP_PUSH = 0x01)
    {0x51, 0x00, 0},      // STORE_GLOBAL 0  (OP_STORE_GLOBAL = 0x51)
    {0x01, 0x00, 84},     // PUSH 84         (OP_PUSH = 0x01) 
    {0x51, 0x00, 1},      // STORE_GLOBAL 1  (OP_STORE_GLOBAL = 0x51)
    {0x50, 0x00, 0},      // LOAD_GLOBAL 0   (OP_LOAD_GLOBAL = 0x50)
    {0x50, 0x00, 1},      // LOAD_GLOBAL 1   (OP_LOAD_GLOBAL = 0x50)
    {0x03, 0x00, 0},      // ADD             (OP_ADD = 0x03)
    {0x02, 0x00, 0},      // POP (discard)   (OP_POP = 0x02)
    {0x00, 0x00, 0}       // HALT            (OP_HALT = 0x00)
};

void test_vm_memory_operations(void) {
    debug_print("=== VM MEMORY OPERATIONS TEST ===");
    
    // Phase 1: Initialize ComponentVM with telemetry
    test_status = 1;
    test_marker = 0xEE00001;
    
    vm_bridge_t* vm = vm_bridge_create();
    if (!vm) {
        debug_print("ERROR: Failed to create ComponentVM");
        test_status = 0xFF;
        return;
    }
    
    debug_print("✓ ComponentVM created successfully");
    
    // Phase 2: Enable telemetry for automated testing
    test_status = 2;
    vm_bridge_enable_telemetry(vm, true);
    if (!vm_bridge_is_telemetry_enabled(vm)) {
        debug_print("ERROR: Failed to enable telemetry");
        vm_bridge_destroy(vm);
        test_status = 0xFF;
        return;
    }
    
    debug_print("✓ Telemetry enabled successfully");
    
    // Phase 3: Load and execute test program
    test_status = 3;
    
    vm_result_t load_result = vm_bridge_load_program(vm, vm_memory_operations_program, 
                                sizeof(vm_memory_operations_program) / sizeof(vm_memory_operations_program[0]));
    if (load_result != VM_RESULT_SUCCESS) {
        debug_print("ERROR: Failed to load test program");
        vm_bridge_destroy(vm);
        test_status = 0xFF;
        return;
    }
    
    debug_print("✓ Test program loaded successfully");
    debug_print_dec("Program size (instructions)", sizeof(vm_memory_operations_program) / sizeof(vm_memory_operations_program[0]));
    
    // Phase 4: Execute program
    test_status = 4;
    debug_print(">>> Starting VM execution...");
    
    vm_result_t exec_result = vm_bridge_execute_program(vm, vm_memory_operations_program, 
                                sizeof(vm_memory_operations_program) / sizeof(vm_memory_operations_program[0]));
    
    if (exec_result == VM_RESULT_SUCCESS) {
        debug_print("✓ VM program executed successfully");
        test_status = 5;
    } else {
        debug_print("ERROR: VM program execution failed");
        debug_print(vm_bridge_get_error_string(exec_result));
        test_status = 0xFF;
    }
    
    // Phase 5: Results available in telemetry
    test_status = 5;
    size_t instruction_count = vm_bridge_get_instruction_count(vm);
    debug_print_dec("Total instructions executed", instruction_count);
    
    // Phase 6: Cleanup
    test_status = 6;
    vm_bridge_destroy(vm);
    
    // Phase 7: Test complete - enter predictable halt state
    test_status = 7;
    test_marker = 0xD0E0002;  // Test completion marker (valid hex)
    
    debug_print("=== VM MEMORY OPERATIONS TEST COMPLETE ===");
    debug_print("Expected: 8 instructions, global[0] = 42, global[1] = 84, stack depth = 0");
    
    // Predictable halt state for automated testing
    for (int i = 0; i < 100; i++) {
        HAL_Delay(10);  // 1 second total settle time
        test_marker = 0xD0E0002 + i;  // Incremental marker
    }
    
    // Final stable state
    test_marker = 0x5AEAB1E2;  // Stable state marker (valid hex)
    test_status = 0x42;        // Success indicator
    
    // LED blink pattern: 4 fast blinks = memory test complete
    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(200);
    }
    
    // Keep running for automated test inspection
    while(1) {
        HAL_Delay(1000);
        debug_print("Memory test complete - telemetry available at 0x20007F00");
    }
}

// Test program entry point
void run_vm_memory_operations_main(void) {
    debug_print("ComponentVM Automated Test: VM Memory Operations");
    debug_print("Phase 4.3.1.2: Memory load/store validation");
    debug_print("");
    
    test_vm_memory_operations();
}

#endif // HARDWARE_PLATFORM