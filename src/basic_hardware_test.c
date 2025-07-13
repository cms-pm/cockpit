/*
 * Basic Hardware Test - LED Only
 * Phase 4.3.3: Minimal hardware validation without VM
 * 
 * This test validates basic hardware setup and LED control
 * without any VM operations to isolate hardware issues.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
#endif

#ifdef HARDWARE_PLATFORM

void run_basic_hardware_test_main(void) {
    // Test 1: Basic LED blink test (no VM involved)
    // This should work regardless of VM state
    
    // Flash LED 5 times quickly to indicate test start
    for(int i = 0; i < 5; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(100);   // 100ms ON
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(100);   // 100ms OFF
    }
    
    // Pause
    HAL_Delay(500);
    
    // Continuous slow blink to indicate hardware is working
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(500);   // 500ms ON (slow blink)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(500);   // 500ms OFF (slow blink)
        
        // This proves:
        // 1. Hardware setup is working
        // 2. GPIO control is functional  
        // 3. HAL_Delay is working
        // 4. Main loop is running continuously
    }
}

#endif // HARDWARE_PLATFORM