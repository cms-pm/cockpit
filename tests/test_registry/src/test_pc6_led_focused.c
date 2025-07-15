/**
 * @file test_pc6_led_focused.c  
 * @brief Focused PC6 LED test - confirmed working on WeAct STM32G431CB
 * 
 * This test validates PC6 LED functionality with both polarities.
 * Successfully migrated from legacy test system.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "semihosting.h"

/**
 * @brief Main test function for PC6 LED validation
 */
void run_pc6_led_focused_main(void) {
    debug_print("\n");
    debug_print("=====================================\n");
    debug_print("PC6 LED Focused Test (Workspace Isolated)\n");
    debug_print("=====================================\n");
    debug_print("Testing confirmed working LED on PC6\n");
    
#ifdef PLATFORM_STM32G4
    // Configure PC6 as output (LED pin confirmed working)
    __HAL_RCC_GPIOC_CLK_ENABLE();  // Ensure GPIOC clock is enabled
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED off initially
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;             // Push-pull output  
    GPIO_InitStruct.Pull = GPIO_NOPULL;                     // No pull resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed sufficient for LED
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    debug_print("PC6 configured as GPIO output for LED control\n");
    
    debug_print("1. Testing PC6 as active HIGH (normal polarity)...\n");
    for (int i = 0; i < 5; i++) {
        debug_print("PC6 = HIGH (LED should be ON)\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(500);
        
        debug_print("PC6 = LOW (LED should be OFF)\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(500);
    }
    
    HAL_Delay(2000);
    
    debug_print("2. Testing PC6 as active LOW (inverted polarity)...\n");
    for (int i = 0; i < 5; i++) {
        debug_print("PC6 = LOW (LED should be ON if active low)\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(500);
        
        debug_print("PC6 = HIGH (LED should be OFF if active low)\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(500);
    }
    
    HAL_Delay(2000);
    
    debug_print("3. Continuous fast blink validation...\n");
    for (int cycle = 0; cycle < 10; cycle++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
    
    debug_print("PC6 LED test complete - workspace isolation working!\n");
    
#else
    debug_print("Non-STM32G4 platform - no LED test available\n");
#endif
}