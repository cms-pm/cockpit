/*
 * UART Circular Buffer Implementation
 * 
 * Thread-safe circular buffer operations for interrupt-driven UART RX.
 * Optimized for single writer (ISR) and single reader (main thread) pattern.
 */

#include "uart_circular_buffer.h"
#include <string.h>

// =================================================================
// Circular Buffer Implementation
// =================================================================

void uart_circular_buffer_init(uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return;
    
    // Initialize all fields to zero/false
    memset((void*)buffer, 0, sizeof(uart_rx_circular_buffer_t));
}

bool uart_circular_buffer_put(uart_rx_circular_buffer_t* buffer, uint8_t data)
{
    if (!buffer) return false;
    
    // Check for buffer overflow (atomic load)
    if (atomic_load(&buffer->count) >= UART_RX_BUFFER_SIZE) {
        buffer->overflow = true;
        return false;  // Buffer full
    }
    
    // Store data at head position
    buffer->buffer[buffer->head] = data;
    
    // Update head with wrap-around (bitwise AND for power-of-2 buffer)
    buffer->head = (buffer->head + 1) & UART_RX_BUFFER_MASK;
    
    // Increment count (atomic operation for ISR safety)
    atomic_fetch_add(&buffer->count, 1);
    
    return true;
}

bool uart_circular_buffer_get(uart_rx_circular_buffer_t* buffer, uint8_t* data)
{
    if (!buffer || !data) return false;
    
    // Check if buffer is empty (atomic load)
    if (atomic_load(&buffer->count) == 0) {
        return false;
    }
    
    // Retrieve data from tail position
    *data = buffer->buffer[buffer->tail];
    
    // Update tail with wrap-around (bitwise AND for power-of-2 buffer)
    buffer->tail = (buffer->tail + 1) & UART_RX_BUFFER_MASK;
    
    // Decrement count (atomic operation for ISR safety)
    atomic_fetch_sub(&buffer->count, 1);
    
    return true;
}

uint16_t uart_circular_buffer_available(const uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return 0;
    
    return atomic_load(&buffer->count);
}

bool uart_circular_buffer_is_empty(const uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return true;
    
    return (atomic_load(&buffer->count) == 0);
}

bool uart_circular_buffer_has_overflow(const uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return false;
    
    return buffer->overflow;
}

void uart_circular_buffer_clear_overflow(uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return;
    
    buffer->overflow = false;
}

void uart_circular_buffer_flush(uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return;
    
    // Reset all pointers and counters
    buffer->head = 0;
    buffer->tail = 0;
    atomic_store(&buffer->count, 0);
    buffer->overflow = false;
}