/*
 * Simple SysTick Test - Diagnose HAL_Delay Issue
 * Phase 4.3.3: Hardware execution debugging
 * 
 * This test validates basic GPIO without using HAL_Delay
 * to isolate the SysTick timer issue.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
#endif

#ifdef HARDWARE_PLATFORM

// Simple delay using busy loop instead of HAL_Delay
void simple_delay_ms(uint32_t ms) {
    // Rough approximation: 170MHz CPU, ~170 cycles per millisecond
    volatile uint32_t cycles = ms * 42500; // Conservative estimate
    while(cycles--) {
        __NOP(); // No operation - prevents optimization
    }
}

void run_simple_systick_test_main(void) {
    // Test 1: Basic LED operation without HAL_Delay
    // Flash LED 3 times quickly to indicate test start
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        simple_delay_ms(100);   // Use simple delay instead of HAL_Delay
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        simple_delay_ms(100);   // Use simple delay instead of HAL_Delay
    }
    
    // Brief pause
    simple_delay_ms(500);
    
    // Test 2: Continuous slow blink to indicate success
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        simple_delay_ms(300);   
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        simple_delay_ms(300);   
        
        // Pattern: Slow blink = Simple GPIO + timing works, SysTick is the issue
    }
}

#endif // HARDWARE_PLATFORM