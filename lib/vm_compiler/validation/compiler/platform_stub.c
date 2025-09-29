/*
 * Minimal Platform Stub for Compiler Validation
 * Provides stub implementations of platform functions needed for VM operation
 * without requiring actual hardware or complex platform initialization
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Platform initialization stub
void platform_init(void) {
    // Validation platform - no initialization needed
}

// GPIO stub functions
bool platform_gpio_config(uint8_t pin, uint8_t mode, uint8_t pull) {
    printf("[GPIO] config pin=%d mode=%d pull=%d\n", pin, mode, pull);
    return true;
}

bool platform_gpio_write(uint8_t pin, bool value) {
    printf("[GPIO] write pin=%d value=%d\n", pin, value ? 1 : 0);
    return true;
}

bool platform_gpio_read(uint8_t pin, bool* value) {
    *value = false; // Always return false for validation
    printf("[GPIO] read pin=%d -> %d\n", pin, 0);
    return true;
}

// UART stub functions
bool platform_uart_init(uint32_t baud_rate, uint8_t data_bits, 
                       uint8_t parity, uint8_t stop_bits) {
    printf("[UART] init baud=%u data=%d parity=%d stop=%d\n", 
           baud_rate, data_bits, parity, stop_bits);
    return true;
}

bool platform_uart_transmit(const uint8_t* data, uint16_t size, uint32_t timeout) {
    printf("[UART] tx size=%d: ", size);
    for (uint16_t i = 0; i < size; i++) {
        if (data[i] >= 32 && data[i] < 127) {
            printf("%c", data[i]);
        } else {
            printf("\\x%02X", data[i]);
        }
    }
    printf("\n");
    return true;
}

bool platform_uart_data_available(void) {
    return false; // No data available in validation mode
}

bool platform_uart_receive(uint8_t* data, uint16_t max_size, uint16_t* received_size, uint32_t timeout) {
    *received_size = 0;
    return true;
}

// Debug UART stub functions
bool platform_debug_uart_init(uint32_t baud_rate, uint8_t data_bits,
                              uint8_t parity, uint8_t stop_bits) {
    printf("[DEBUG_UART] init baud=%u data=%d parity=%d stop=%d\n", 
           baud_rate, data_bits, parity, stop_bits);
    return true;
}

bool platform_debug_uart_transmit(const uint8_t* data, uint16_t size) {
    printf("[DEBUG_UART] tx: ");
    for (uint16_t i = 0; i < size; i++) {
        if (data[i] >= 32 && data[i] < 127) {
            printf("%c", data[i]);
        } else {
            printf("\\x%02X", data[i]);
        }
    }
    printf("\n");
    return true;
}

// Timing stub functions
void platform_delay_ms(uint32_t ms) {
    printf("[TIMING] delay %u ms\n", ms);
    // No actual delay in validation mode
}

uint32_t platform_get_tick_ms(void) {
    static uint32_t tick_counter = 0;
    return ++tick_counter; // Simple incrementing counter
}