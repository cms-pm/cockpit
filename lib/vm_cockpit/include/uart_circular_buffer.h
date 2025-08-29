/*
 * UART Circular Buffer - Platform-agnostic interrupt-driven UART RX
 * 
 * Provides thread-safe circular buffer for UART RX data with interrupt support.
 * Designed for bootloader protocol communication with Oracle testing tool.
 */

#ifndef UART_CIRCULAR_BUFFER_H
#define UART_CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// Buffer size configuration - sufficient for max protocol frame (279 bytes + margin)
// MUST be power of 2 for bitwise operations (enforced by static_assert)
#define UART_RX_BUFFER_SIZE 512
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)

// Compile-time validation that buffer size is power of 2
_Static_assert((UART_RX_BUFFER_SIZE & (UART_RX_BUFFER_SIZE - 1)) == 0, 
               "UART_RX_BUFFER_SIZE must be power of 2 for safe bitwise operations");

// Circular buffer structure for interrupt-safe UART RX
typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];  // Data storage
    volatile uint16_t head;               // Write index (ISR updates)
    volatile uint16_t tail;               // Read index (main thread updates)
    volatile _Atomic uint16_t count;      // Number of bytes available (atomic)
    volatile bool overflow;               // Buffer overflow flag
} uart_rx_circular_buffer_t;

// =================================================================
// Circular Buffer Operations (Interrupt-Safe)
// =================================================================

/**
 * @brief Initialize circular buffer
 * @param buffer Pointer to circular buffer structure
 */
void uart_circular_buffer_init(uart_rx_circular_buffer_t* buffer);

/**
 * @brief Add byte to circular buffer (called from ISR)
 * @param buffer Pointer to circular buffer structure
 * @param data Byte to add
 * @return true if byte added successfully, false if overflow
 */
bool uart_circular_buffer_put(uart_rx_circular_buffer_t* buffer, uint8_t data);

/**
 * @brief Get byte from circular buffer (called from main thread)
 * @param buffer Pointer to circular buffer structure
 * @param data Pointer to store retrieved byte
 * @return true if byte retrieved successfully, false if buffer empty
 */
bool uart_circular_buffer_get(uart_rx_circular_buffer_t* buffer, uint8_t* data);

/**
 * @brief Check if data is available in buffer
 * @param buffer Pointer to circular buffer structure
 * @return Number of bytes available
 */
uint16_t uart_circular_buffer_available(const uart_rx_circular_buffer_t* buffer);

/**
 * @brief Check if buffer is empty
 * @param buffer Pointer to circular buffer structure
 * @return true if buffer is empty
 */
bool uart_circular_buffer_is_empty(const uart_rx_circular_buffer_t* buffer);

/**
 * @brief Check if buffer overflow occurred
 * @param buffer Pointer to circular buffer structure
 * @return true if overflow occurred since last reset
 */
bool uart_circular_buffer_has_overflow(const uart_rx_circular_buffer_t* buffer);

/**
 * @brief Reset buffer overflow flag
 * @param buffer Pointer to circular buffer structure
 */
void uart_circular_buffer_clear_overflow(uart_rx_circular_buffer_t* buffer);

/**
 * @brief Flush all data from buffer
 * @param buffer Pointer to circular buffer structure
 */
void uart_circular_buffer_flush(uart_rx_circular_buffer_t* buffer);

#ifdef __cplusplus
}
#endif

#endif // UART_CIRCULAR_BUFFER_H