/*
 * VM Cockpit Host Interface - UART Operations
 * Platform-agnostic UART functions using common platform interface
 */

#include "host_interface.h"
#include "../platform/platform_interface.h"

// =================================================================
// UART Operations Implementation
// =================================================================

void uart_begin(uint32_t baud_rate) {
    platform_uart_init(baud_rate);
}

void uart_write_string(const char* str) {
    if (str == NULL) {
        return;
    }
    
    // Calculate string length
    uint16_t length = 0;
    const char* ptr = str;
    while (*ptr++) {
        length++;
    }
    
    if (length > 0) {
        platform_uart_transmit((const uint8_t*)str, length);
    }
}

void uart_write_char(char c) {
    uint8_t data = (uint8_t)c;
    platform_uart_transmit(&data, 1);
}

void uart_write_frame(const uint8_t* frame_data, uint16_t frame_length) {
    if (frame_data == NULL || frame_length == 0) {
        return;
    }
    
    // Atomic frame transmission via STM32 HAL
    platform_uart_transmit(frame_data, frame_length);
    
    // Flush the transmit buffer
    platform_uart_transmit(NULL, 0);
}

bool uart_data_available(void) {
    return platform_uart_data_available();
}

char uart_read_char(void) {
    uint8_t data = 0;
    platform_result_t result = platform_uart_receive(&data);
    
    if (result == PLATFORM_OK) {
        return (char)data;
    }
    
    return 0;
}

// =================================================================
// Debug UART Operations Implementation
// =================================================================

void debug_uart_begin(uint32_t baud_rate) {
    platform_debug_uart_init(baud_rate);
}

void debug_uart_write_string(const char* str) {
    if (str == NULL) {
        return;
    }
    
    // Calculate string length
    uint16_t length = 0;
    const char* ptr = str;
    while (*ptr++) {
        length++;
    }
    
    if (length > 0) {
        platform_debug_uart_transmit((const uint8_t*)str, length);
    }
}

void debug_uart_write_data(const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0) {
        return;
    }
    
    platform_debug_uart_transmit(data, length);
}