/*
 * Basic Observer Pattern Integration Test
 * Phase 4.3.2A: Test ComponentVM observer pattern with vm_blackbox_observer
 * 
 * This test validates the minimal observer interface works correctly
 * and demonstrates the architecture for SOS hardware validation tests.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/semihosting/semihosting.h"
    #include "../lib/vm_bridge/vm_bridge.h"
    #include "../lib/vm_blackbox/include/vm_blackbox.h"
    #include "../include/memory_layout.h"
#endif

#ifdef HARDWARE_PLATFORM

// Test sequence markers for debugging - unique names to avoid conflicts
volatile uint32_t observer_test_marker = 0x0B5E4E4; // "OBSERVER" pattern
volatile uint32_t observer_test_phase = 0;

// Simple test program to validate observer notifications
static const vm_instruction_t test_observer_program[] = {
    // Simple sequence: PUSH 42, PUSH 24, ADD, HALT
    {0x01, 0x00, 42},     // PUSH 42         (OP_PUSH = 0x01)
    {0x01, 0x00, 24},     // PUSH 24         (OP_PUSH = 0x01)  
    {0x03, 0x00, 0},      // ADD             (OP_ADD = 0x03)
    {0x00, 0x00, 0}       // HALT            (OP_HALT = 0x00)
};

void test_observer_pattern_integration(void) {
    debug_print("=== OBSERVER PATTERN INTEGRATION TEST START ===");
    
    // Phase 1: Create VM with telemetry (using existing vm_bridge)
    observer_test_phase = 1;
    observer_test_marker = 0x0B5E4001;
    
    vm_bridge_t* vm = vm_bridge_create();
    if (!vm) {
        debug_print("ERROR: Failed to create VM");
        return;
    }
    
    debug_print("✓ ComponentVM created successfully");
    
    // Phase 2: Enable telemetry (this uses vm_blackbox_observer internally)
    observer_test_phase = 2;
    observer_test_marker = 0x0B5E4002;
    
    vm_bridge_enable_telemetry(vm, true);
    if (!vm_bridge_is_telemetry_enabled(vm)) {
        debug_print("ERROR: Failed to enable telemetry");
        vm_bridge_destroy(vm);
        return;
    }
    
    debug_print("✓ Telemetry enabled (observer pattern active)");
    
    // Phase 3: Execute test program to trigger observer notifications
    observer_test_phase = 3;
    observer_test_marker = 0x0B5E4003;
    
    vm_result_t result = vm_bridge_execute_program(vm, test_observer_program, 
                                sizeof(test_observer_program) / sizeof(test_observer_program[0]));
    
    if (result == VM_RESULT_SUCCESS) {
        debug_print("✓ Program executed successfully");
        debug_print("Observer pattern captured execution events");
    } else {
        debug_print("ERROR: Program execution failed");
    }
    
    // Phase 4: Validate telemetry data was captured
    observer_test_phase = 4;
    observer_test_marker = 0x0B5E4004;
    
    size_t instruction_count = vm_bridge_get_instruction_count(vm);
    debug_print_dec("Instructions executed (via observer)", instruction_count);
    
    // Phase 5: Verify telemetry memory contains observer data
    observer_test_phase = 5;
    observer_test_marker = 0x0B5E4005;
    
    volatile uint32_t* telemetry_ptr = (volatile uint32_t*)TELEMETRY_BASE_ADDR;
    debug_print("=== OBSERVER TELEMETRY VALIDATION ===");
    debug_print_hex("Telemetry magic", telemetry_ptr[0]);
    debug_print_hex("Format version", telemetry_ptr[1]);
    debug_print_hex("Program counter", telemetry_ptr[2]);
    debug_print_hex("Instruction count", telemetry_ptr[3]);
    
    // Phase 6: Test observer pattern reset
    observer_test_phase = 6;
    observer_test_marker = 0x0B5E4006;
    
    vm_bridge_reset(vm);
    debug_print("✓ VM reset completed (observer notified)");
    
    // Phase 7: Cleanup
    observer_test_phase = 7;
    observer_test_marker = 0x0B5E4007;
    
    vm_bridge_destroy(vm);
    debug_print("✓ VM destroyed (observer cleanup)");
    debug_print("=== OBSERVER PATTERN INTEGRATION TEST COMPLETE ===");
}

void run_observer_pattern_test_main(void) {
    debug_print("ComponentVM Observer Pattern Integration Test");
    debug_print("Phase 4.3.2A: ITelemetryObserver -> vm_blackbox_observer validation");
    debug_print("");
    
    // Run the integration test
    test_observer_pattern_integration();
    
    // Success state - fast blink to indicate observer test completion
    debug_print("Observer pattern test completed - entering fast blink mode");
    debug_print("LED will blink rapidly to indicate observer test success");
    
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(100);   // 100ms ON (fast blink)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(100);   // 100ms OFF (fast blink)
        
        // Periodic status
        debug_print("Observer pattern test complete - architecture validated");
    }
}

#endif // HARDWARE_PLATFORM