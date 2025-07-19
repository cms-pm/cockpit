/*
 * QEMU Platform Implementation
 * Platform Layer 4 implementation for QEMU virtual hardware
 */

#include "qemu_platform.h"
#include "qemu_semihosting.h"
#include <stddef.h>

#ifdef QEMU_PLATFORM

// =================================================================
// Platform Interface Implementation for QEMU
// =================================================================

void qemu_platform_init(void) {
    // Initialize semihosting (QEMU's HAL equivalent)
    qemu_semihost_init();
    
    // Platform-specific initialization complete
    qemu_semihost_puts("QEMU Platform Layer Initialized\n");
}

// =================================================================
// Platform Interface Functions (implements platform_interface.h)
// =================================================================

void platform_init(void) {
    qemu_platform_init();
}

void platform_delay_ms(uint32_t milliseconds) {
    qemu_semihost_delay_ms(milliseconds);
}

uint32_t platform_get_tick_ms(void) {
    return qemu_semihost_get_time_ms();
}

platform_result_t platform_gpio_config(uint8_t logical_pin, platform_gpio_mode_t mode) {
    // Configure virtual GPIO direction based on mode
    bool is_output = (mode == PLATFORM_GPIO_OUTPUT);
    qemu_gpio_set_direction(logical_pin, is_output);
    
    // Set initial state for inputs with pull resistors
    if (mode == PLATFORM_GPIO_INPUT_PULLUP) {
        qemu_gpio_set_pin(logical_pin, true);  // Pulled high
    } else if (mode == PLATFORM_GPIO_INPUT_PULLDOWN) {
        qemu_gpio_set_pin(logical_pin, false); // Pulled low
    }
    
    return PLATFORM_OK;
}

platform_result_t platform_gpio_write(uint8_t logical_pin, platform_gpio_state_t state) {
    bool pin_state = (state == PLATFORM_GPIO_HIGH);
    qemu_gpio_set_pin(logical_pin, pin_state);
    return PLATFORM_OK;
}

platform_result_t platform_gpio_read(uint8_t logical_pin, platform_gpio_state_t* state) {
    if (state == NULL) return PLATFORM_INVALID_PARAM;
    
    bool pin_state = qemu_gpio_get_pin(logical_pin);
    *state = pin_state ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW;
    return PLATFORM_OK;
}

platform_result_t platform_uart_init(uint32_t baud_rate) {
    // QEMU UART initialization - semihosting is always "ready"
    char msg[64];
    qemu_semihost_puts("QEMU UART: Initialized at ");
    
    // Simple number to string conversion for baud rate
    uint32_t temp = baud_rate;
    char digits[16];
    int digit_count = 0;
    
    if (temp == 0) {
        digits[digit_count++] = '0';
    } else {
        while (temp > 0) {
            digits[digit_count++] = '0' + (temp % 10);
            temp /= 10;
        }
    }
    
    // Reverse digits and print
    for (int i = digit_count - 1; i >= 0; i--) {
        qemu_semihost_putchar(digits[i]);
    }
    qemu_semihost_puts(" baud\n");
    
    return PLATFORM_OK;
}

platform_result_t platform_uart_transmit(const uint8_t* data, uint16_t size) {
    if (data == NULL || size == 0) return PLATFORM_INVALID_PARAM;
    
    qemu_semihost_write(data, size);
    return PLATFORM_OK;
}

bool platform_uart_data_available(void) {
    // In most QEMU setups, input is not readily available
    // This would need to be enhanced for interactive QEMU usage
    return false;
}

platform_result_t platform_uart_receive(uint8_t* data) {
    if (data == NULL) return PLATFORM_INVALID_PARAM;
    
    int received = qemu_semihost_getchar();
    if (received >= 0) {
        *data = (uint8_t)received;
        return PLATFORM_OK;
    }
    
    return PLATFORM_TIMEOUT;
}

// =================================================================
// QEMU-Specific Platform Functions
// =================================================================

void qemu_gpio_simulate_state(uint8_t pin, bool state) {
    qemu_gpio_set_pin(pin, state);
}

bool qemu_gpio_get_simulated_state(uint8_t pin) {
    return qemu_gpio_get_pin(pin);
}

void qemu_uart_semihost_output(const uint8_t* data, uint16_t size) {
    qemu_semihost_write(data, size);
}

#endif // QEMU_PLATFORM