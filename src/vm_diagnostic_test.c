/*
 * VM Diagnostic Test - Isolate VM Bridge Failure Point
 * Phase 4.3.3: Hardware execution debugging
 * 
 * This test adds LED breadcrumbs to isolate exactly where VM execution fails
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/vm_bridge/vm_bridge.h"
#endif

#ifdef HARDWARE_PLATFORM

// LED breadcrumb helper
void led_breadcrumb(int flashes, int delay_ms) {
    for(int i = 0; i < flashes; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(delay_ms);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(delay_ms);
    }
    HAL_Delay(300); // Pause between breadcrumbs
}

void run_vm_diagnostic_test_main(void) {
    // Breadcrumb 1: Program started
    led_breadcrumb(1, 100); // 1 flash = program start
    
    // Breadcrumb 2: About to create VM
    led_breadcrumb(2, 100); // 2 flashes = about to create VM
    
    vm_bridge_t* vm = vm_bridge_create();
    
    // Breadcrumb 3: VM creation result
    if (vm != NULL) {
        led_breadcrumb(3, 100); // 3 flashes = VM created successfully
    } else {
        led_breadcrumb(9, 50);  // 9 fast flashes = VM creation failed
        while(1) {} // Halt here for debugging
    }
    
    // Breadcrumb 4: About to execute program
    led_breadcrumb(4, 100); // 4 flashes = about to execute
    
    const vm_instruction_t test_program[] = {
        {0x01, 0x00, 42},     // PUSH 42
        {0x01, 0x00, 24},     // PUSH 24  
        {0x03, 0x00, 0},      // ADD
        {0x00, 0x00, 0}       // HALT
    };
    
    vm_result_t result = vm_bridge_execute_program(vm, test_program, 4);
    
    // Breadcrumb 5: Execution result
    if (result == VM_RESULT_SUCCESS) {
        led_breadcrumb(5, 100); // 5 flashes = execution success
    } else {
        led_breadcrumb(8, 50);  // 8 fast flashes = execution failed
    }
    
    // Breadcrumb 6: About to cleanup
    led_breadcrumb(6, 100); // 6 flashes = about to cleanup
    
    if (vm != NULL) {
        vm_bridge_destroy(vm);
    }
    
    // Breadcrumb 7: All done, continuous success indicator
    led_breadcrumb(7, 100); // 7 flashes = all complete
    
    // Final result: Slow blink = all tests passed
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(500);
    }
}

#endif // HARDWARE_PLATFORM