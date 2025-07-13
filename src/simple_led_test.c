/*
 * Simple LED Test - Hardware Validation
 * Phase 4.3.3: Basic hardware execution validation without semihosting
 * 
 * This test validates that firmware uploads and executes on STM32G431CB
 * LED behavior indicates successful execution and VM operation.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/vm_bridge/vm_bridge.h"
#endif

#ifdef HARDWARE_PLATFORM

void run_simple_led_test_main(void) {
    // Test 1: Basic LED operation (validate hardware setup)
    // Flash LED 3 times quickly to indicate test start
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(50);   // 50ms ON
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(50);   // 50ms OFF
    }
    
    // Brief pause
    HAL_Delay(500);
    
    // Test 2: VM Bridge operation
    vm_bridge_t* vm = vm_bridge_create();
    bool vm_created = (vm != NULL);
    
    // Simple test program: PUSH 42, PUSH 24, ADD, HALT
    const vm_instruction_t test_program[] = {
        {0x01, 0x00, 42},     // PUSH 42
        {0x01, 0x00, 24},     // PUSH 24  
        {0x03, 0x00, 0},      // ADD
        {0x00, 0x00, 0}       // HALT
    };
    
    // Execute test program
    vm_result_t result = VM_RESULT_ERROR;
    if (vm_created) {
        result = vm_bridge_execute_program(vm, test_program, 4);
    }
    
    // Test 3: Cleanup
    if (vm != NULL) {
        vm_bridge_destroy(vm);
    }
    
    // Test 4: Results indication via LED pattern
    // Success: Medium blink (200ms on/off)
    // Failure: Fast blink (100ms on/off)
    
    bool test_success = (vm_created && result == VM_RESULT_SUCCESS);
    uint32_t blink_delay = test_success ? 200 : 100;
    
    // Continuous LED pattern to indicate test results
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(blink_delay);   
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(blink_delay);   
        
        // LED Pattern Legend:
        // Medium blink (200ms) = SUCCESS: VM created and executed program
        // Fast blink (100ms) = FAILURE: VM creation or execution failed
    }
}

#endif // HARDWARE_PLATFORM