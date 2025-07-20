/*
 * VM Cockpit Host Interface - GPIO Operations
 * Platform-agnostic GPIO functions using common platform interface
 */

#include "host_interface.h"
#include "../platform/platform_interface.h"

// =================================================================
// GPIO Operations Implementation
// =================================================================

void gpio_pin_config(uint8_t pin, gpio_mode_t mode) {
    platform_gpio_mode_t platform_mode;
    
    // Translate host interface GPIO mode to platform mode
    switch (mode) {
        case GPIO_INPUT:
            platform_mode = PLATFORM_GPIO_INPUT;
            break;
        case GPIO_OUTPUT:
            platform_mode = PLATFORM_GPIO_OUTPUT;
            break;
        case GPIO_INPUT_PULLUP:
            platform_mode = PLATFORM_GPIO_INPUT_PULLUP;
            break;
        case GPIO_INPUT_PULLDOWN:
            platform_mode = PLATFORM_GPIO_INPUT_PULLDOWN;
            break;
        default:
            return; // Invalid mode
    }
    
    // Use common platform interface
    platform_gpio_config(pin, platform_mode);
}

void gpio_pin_write(uint8_t pin, bool state) {
    platform_gpio_state_t platform_state = state ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW;
    platform_gpio_write(pin, platform_state);
}

bool gpio_pin_read(uint8_t pin) {
    platform_gpio_state_t platform_state;
    platform_result_t result = platform_gpio_read(pin, &platform_state);
    
    if (result != PLATFORM_OK) {
        return false; // Error reading pin
    }
    
    return (platform_state == PLATFORM_GPIO_HIGH);
}