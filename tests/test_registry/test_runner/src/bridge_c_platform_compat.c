/**
 * Bridge C Platform Compatibility Layer
 * Provides the specific function names that bridge_c expects
 * while delegating to the vm_compiler platform_stub functions
 */

#include <stdint.h>
#include <stdbool.h>

// Forward declarations for vm_compiler platform functions
extern void platform_delay_ms(uint32_t ms);
extern uint32_t platform_get_tick_ms(void);
extern bool platform_gpio_write(uint8_t pin, bool value);
extern bool platform_gpio_read(uint8_t pin, bool* value);

// Bridge C compatibility functions
void gpio_pin_write(uint8_t pin, uint8_t value) {
    platform_gpio_write(pin, value != 0);
}

uint8_t gpio_pin_read(uint8_t pin) {
    bool value = false;
    platform_gpio_read(pin, &value);
    return value ? 1 : 0;
}

void delay_ms(uint32_t ms) {
    platform_delay_ms(ms);
}

uint32_t get_tick_ms(void) {
    return platform_get_tick_ms();
}

uint32_t get_tick_us(void) {
    // Simple conversion - vm_compiler stub doesn't provide microsecond precision
    return platform_get_tick_ms() * 1000;
}