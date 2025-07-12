/*
 * STM32G4 HAL Test Program
 * Tests the platform-aware Arduino HAL implementation
 */

#ifdef HARDWARE_PLATFORM

#include "../lib/arduino_hal/arduino_hal.h"
#include "../lib/semihosting/semihosting.h"

void test_stm32g4_hal(void) {
    debug_print("=== STM32G4 HAL Test Starting ===");
    
    // Initialize HAL
    hal_gpio_init();
    
    // Test LED pin configuration (Arduino pin 13 = PC6)
    debug_print("Configuring LED pin (Arduino 13)");
    hal_gpio_set_mode(13, PIN_MODE_OUTPUT);
    
    // Test GPIO write operations
    debug_print("Testing GPIO write operations");
    for (int i = 0; i < 5; i++) {
        hal_gpio_write(13, PIN_HIGH);
        debug_print("LED ON");
        arduino_delay(200);
        
        hal_gpio_write(13, PIN_LOW);
        debug_print("LED OFF");
        arduino_delay(200);
    }
    
    // Test button input (Arduino pin 2 = PA2)
    debug_print("Configuring button pin (Arduino 2)");
    hal_gpio_set_mode(2, PIN_MODE_INPUT_PULLUP);
    
    // Test GPIO read operations
    debug_print("Testing GPIO read operations");
    for (int i = 0; i < 10; i++) {
        pin_state_t button_state = hal_gpio_read(2);
        debug_print_dec("Button state", button_state);
        arduino_delay(100);
    }
    
    debug_print("=== STM32G4 HAL Test Complete ===");
}

#endif // HARDWARE_PLATFORM