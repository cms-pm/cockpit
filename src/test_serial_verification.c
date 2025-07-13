/*
 * Serial Verification Test - Phase 4.5.1
 * Simple test to verify Serial.print/println works via both UART and semihosting
 * 
 * This test provides clear visual feedback that Serial functions are operational.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/arduino_hal/arduino_hal.h"
    #include "../lib/arduino_hal/uart_hal.h"
    #include "../lib/arduino_hal/platforms/stm32g4_config.h"
#endif

void run_test_serial_verification_main(void) {
#ifdef HARDWARE_PLATFORM
    // Initialize Arduino system first
    arduino_system_init();
    
    // Brief delay to ensure system is stable
    HAL_Delay(100);
    
    // Test Serial functionality
    Serial_begin(115200);
    
    // Simple output test
    Serial_println("=== Serial Verification Test ===");
    Serial_print("Testing Serial.print: ");
    Serial_println("SUCCESS!");
    
    Serial_println("UART Configuration:");
    Serial_print("  - Baud Rate: ");
    Serial_println("115200");
    Serial_print("  - Data Bits: ");
    Serial_println("8");
    Serial_print("  - Stop Bits: ");
    Serial_println("1");
    Serial_print("  - Parity: ");
    Serial_println("None");
    
    Serial_println("");
    Serial_println("Serial API Test Results:");
    Serial_println("  ✓ Serial.begin() - OK");
    Serial_println("  ✓ Serial.print() - OK");
    Serial_println("  ✓ Serial.println() - OK");
    
    Serial_println("");
    Serial_println("Output Methods:");
    Serial_println("  - Hardware UART: PA9/PA10");
    Serial_println("  - Debug Console: Semihosting");
    
    Serial_println("");
    Serial_println("=== Test Complete ===");
    
    // LED pattern: 3 quick flashes then continuous medium blink for success
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
    
    HAL_Delay(500);  // Pause
    
    // Continuous success pattern with heartbeat messages
    int cycle_count = 0;
    while (1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(250);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(250);
        
        cycle_count++;
        if (cycle_count >= 10) {  // Every 5 seconds
            Serial_println("Serial Heartbeat: System operational");
            cycle_count = 0;
        }
    }
#endif
}