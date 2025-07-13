/*
 * UART Hardware Abstraction Layer
 * Phase 4.5.1: UART Peripheral Setup for STM32G431CB
 * 
 * Provides both low-level UART functions (bootloader-ready) and Arduino Serial API.
 * Designed for extensibility from blocking I/O to interrupt-driven circular buffers.
 * 
 * Architecture:
 * - Direct HAL functions for bootloader protocols
 * - Arduino Serial API for SOS MVP compatibility  
 * - Register validation for confident hardware debugging
 * - Clean upgrade path to interrupt-driven operation
 */

#ifndef UART_HAL_H
#define UART_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// UART Result Codes - Fail-fast error handling for embedded systems
typedef enum {
    UART_SUCCESS = 0,
    UART_ERROR_INIT,
    UART_ERROR_TIMEOUT, 
    UART_ERROR_HARDWARE,
    UART_ERROR_INVALID_PARAM
} uart_result_t;

// UART Configuration Structure
typedef struct {
    uint32_t baud_rate;
    bool initialized;
    uint32_t timeout_ms;  // Blocking operation timeout
} uart_config_t;

// =================================================================
// Low-Level UART HAL Functions (Bootloader-Ready)
// =================================================================

/**
 * Initialize UART hardware with specified baud rate
 * 
 * @param baud_rate Communication speed (115200, 9600, etc.)
 * @return UART_SUCCESS on success, error code on failure
 */
uart_result_t uart_init(uint32_t baud_rate);

/**
 * Send single character via UART (blocking)
 * 
 * @param c Character to transmit
 * @return UART_SUCCESS on success, UART_ERROR_TIMEOUT on timeout
 */
uart_result_t uart_putchar(char c);

/**
 * Send null-terminated string via UART (blocking)
 * 
 * @param str String to transmit
 * @return UART_SUCCESS on success, error code on failure
 */
uart_result_t uart_write_string(const char* str);

/**
 * Send binary data via UART (blocking)
 * 
 * @param data Buffer to transmit
 * @param length Number of bytes to send
 * @return UART_SUCCESS on success, error code on failure
 */
uart_result_t uart_write_bytes(const uint8_t* data, size_t length);

/**
 * Check if received data is available
 * 
 * @return true if data available, false otherwise
 */
bool uart_data_available(void);

/**
 * Receive single character via UART (blocking)
 * 
 * @return Received character, or 0 on timeout/error
 */
char uart_getchar(void);

/**
 * Get UART configuration and status
 * 
 * @return Pointer to current UART configuration
 */
const uart_config_t* uart_get_config(void);

// =================================================================
// Arduino Serial API (SOS MVP Compatibility)
// =================================================================

/**
 * Initialize Serial communication (Arduino-style)
 * 
 * @param baud_rate Communication speed
 */
void Serial_begin(uint32_t baud_rate);

/**
 * Print string without line ending (Arduino-style)
 * 
 * @param str String to print
 */
void Serial_print(const char* str);

/**
 * Print string with line ending (Arduino-style)
 * 
 * @param str String to print
 */
void Serial_println(const char* str);

/**
 * Check if Serial is initialized and ready
 * 
 * @return true if ready, false otherwise
 */
bool Serial_ready(void);

// =================================================================
// Register Validation Functions (Development & Debugging)
// =================================================================

/**
 * Validate UART hardware configuration registers
 * Used for hardware debugging and test validation
 * 
 * @return true if all registers configured correctly
 */
bool uart_validate_registers(void);

/**
 * Print detailed UART register states for debugging
 * Output via semihosting/debug interface, not UART itself
 */
void uart_debug_registers(void);

// =================================================================
// Future Extension Hooks (Phase 4.7: Interrupt-Driven)
// =================================================================

// Note: These will be implemented in future phases
// typedef struct uart_buffer_t uart_buffer_t;
// uart_result_t uart_set_interrupt_mode(bool enable);
// uart_result_t uart_set_tx_buffer(uart_buffer_t* buffer);
// uart_result_t uart_set_rx_buffer(uart_buffer_t* buffer);

#endif // UART_HAL_H