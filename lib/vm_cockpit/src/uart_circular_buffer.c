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
    
    // Check for buffer overflow
    if (buffer->count >= UART_RX_BUFFER_SIZE) {
        buffer->overflow = true;
        return false;  // Buffer full
    }
    
    // Store data at head position
    buffer->buffer[buffer->head] = data;
    
    // Update head with wrap-around
    buffer->head = (buffer->head + 1) % UART_RX_BUFFER_SIZE;
    
    // Increment count (atomic for single writer)
    buffer->count++;
    
    return true;
}

bool uart_circular_buffer_get(uart_rx_circular_buffer_t* buffer, uint8_t* data)
{
    if (!buffer || !data) return false;
    
    // Check if buffer is empty
    if (buffer->count == 0) {
        return false;
    }
    
    // Retrieve data from tail position
    *data = buffer->buffer[buffer->tail];
    
    // Update tail with wrap-around
    buffer->tail = (buffer->tail + 1) % UART_RX_BUFFER_SIZE;
    
    // Decrement count (atomic for single reader)
    buffer->count--;
    
    return true;
}

uint16_t uart_circular_buffer_available(const uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return 0;
    
    return buffer->count;
}

bool uart_circular_buffer_is_empty(const uart_rx_circular_buffer_t* buffer)
{
    if (!buffer) return true;
    
    return (buffer->count == 0);
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
    buffer->count = 0;
    buffer->overflow = false;
}