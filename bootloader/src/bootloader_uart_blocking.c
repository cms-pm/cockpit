/*
 * Bootloader Blocking UART Transport Implementation
 * 
 * Simple, reliable blocking UART operations for bootloader foundation.
 * Uses Host Interface layer for hardware abstraction.
 * Designed for deterministic execution and easy debugging.
 */

#include "bootloader_uart_blocking.h"
#include "host_interface/host_interface.h"
#include "bootloader_timeout.h"
#include <string.h>

// Blocking UART context
static blocking_uart_context_t uart_context = {0};

// Transport interface implementation
static transport_status_t blocking_uart_init(void);
static transport_status_t blocking_uart_send(const uint8_t* data, uint16_t len, uint32_t timeout_ms);
static transport_status_t blocking_uart_receive(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms);
static transport_status_t blocking_uart_available(uint16_t* available_bytes);
static transport_status_t blocking_uart_flush(void);
static transport_status_t blocking_uart_deinit(void);
static transport_status_t blocking_uart_get_stats(transport_stats_t* stats);
static const char* blocking_uart_get_name(void);

// Transport interface definition
const transport_interface_t blocking_uart_transport = {
    .init = blocking_uart_init,
    .send = blocking_uart_send,
    .receive = blocking_uart_receive,
    .available = blocking_uart_available,
    .flush = blocking_uart_flush,
    .deinit = blocking_uart_deinit,
    .get_stats = blocking_uart_get_stats,
    .get_name = blocking_uart_get_name
};

static transport_status_t blocking_uart_init(void) {
    // Initialize UART using Host Interface
    uart_begin(BOOTLOADER_UART_BAUD_RATE);
    
    // Initialize context
    memset(&uart_context, 0, sizeof(blocking_uart_context_t));
    uart_context.baud_rate = BOOTLOADER_UART_BAUD_RATE;
    uart_context.initialized = true;
    
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_send(const uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    if (!data || len == 0) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!uart_context.initialized) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    simple_timeout_t timeout;
    timeout_init(&timeout, timeout_ms);
    
    // Blocking send operation
    for (uint16_t i = 0; i < len; i++) {
        // Check timeout before each byte
        if (is_timeout_expired(&timeout)) {
            uart_context.timeout_count++;
            return TRANSPORT_ERROR_TIMEOUT;
        }
        
        // Send byte using Host Interface (inherently blocking)
        uart_write_char(data[i]);
        uart_context.bytes_sent++;
    }
    
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_receive(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms) {
    if (!data || max_len == 0 || !actual_len) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!uart_context.initialized) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    *actual_len = 0;
    simple_timeout_t timeout;
    timeout_init(&timeout, timeout_ms);
    
    // Blocking receive operation
    for (uint16_t i = 0; i < max_len; i++) {
        // Wait for data with timeout
        while (!uart_data_available()) {
            if (is_timeout_expired(&timeout)) {
                uart_context.timeout_count++;
                return (*actual_len > 0) ? TRANSPORT_OK : TRANSPORT_ERROR_TIMEOUT;
            }
            // Small delay to prevent CPU burning
            delay_us(BOOTLOADER_UART_POLL_DELAY_US);
        }
        
        // Read available byte
        data[i] = uart_read_char();
        (*actual_len)++;
        uart_context.bytes_received++;
        
        // For single-byte reads, return immediately
        if (max_len == 1) {
            break;
        }
        
        // Check if more data is immediately available
        if (!uart_data_available()) {
            // No more data immediately available, return what we have
            break;
        }
    }
    
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_available(uint16_t* available_bytes) {
    if (!available_bytes) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!uart_context.initialized) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    // Simple availability check using Host Interface
    *available_bytes = uart_data_available() ? 1 : 0;
    
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_flush(void) {
    if (!uart_context.initialized) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    // For blocking implementation, flush is immediate
    // Host Interface UART operations are inherently blocking
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_deinit(void) {
    // Simple cleanup
    uart_context.initialized = false;
    memset(&uart_context, 0, sizeof(blocking_uart_context_t));
    
    return TRANSPORT_OK;
}

static transport_status_t blocking_uart_get_stats(transport_stats_t* stats) {
    if (!stats) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    stats->bytes_sent = uart_context.bytes_sent;
    stats->bytes_received = uart_context.bytes_received;
    stats->error_count = uart_context.error_count;
    stats->timeout_count = uart_context.timeout_count;
    stats->state = uart_context.initialized ? TRANSPORT_STATE_INITIALIZED : TRANSPORT_STATE_UNINITIALIZED;
    
    return TRANSPORT_OK;
}

static const char* blocking_uart_get_name(void) {
    return "blocking_uart";
}

// Public API functions
bootloader_error_t bootloader_uart_init(void) {
    transport_status_t status = blocking_uart_init();
    return (status == TRANSPORT_OK) ? BOOTLOADER_SUCCESS : BOOTLOADER_ERROR_UART_INIT;
}

bootloader_error_t bootloader_uart_send_bytes(const uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    transport_status_t status = blocking_uart_send(data, len, timeout_ms);
    
    switch (status) {
        case TRANSPORT_OK:
            return BOOTLOADER_SUCCESS;
        case TRANSPORT_ERROR_TIMEOUT:
            return BOOTLOADER_ERROR_UART_TIMEOUT;
        case TRANSPORT_ERROR_INVALID_PARAM:
            return BOOTLOADER_ERROR_INVALID_PARAM;
        default:
            return BOOTLOADER_ERROR_UART_HARDWARE;
    }
}

bootloader_error_t bootloader_uart_receive_bytes(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms) {
    transport_status_t status = blocking_uart_receive(data, max_len, actual_len, timeout_ms);
    
    switch (status) {
        case TRANSPORT_OK:
            return BOOTLOADER_SUCCESS;
        case TRANSPORT_ERROR_TIMEOUT:
            return BOOTLOADER_ERROR_UART_TIMEOUT;
        case TRANSPORT_ERROR_INVALID_PARAM:
            return BOOTLOADER_ERROR_INVALID_PARAM;
        default:
            return BOOTLOADER_ERROR_UART_HARDWARE;
    }
}

bool bootloader_uart_data_available(void) {
    if (!uart_context.initialized) {
        return false;
    }
    
    return uart_data_available();
}

const transport_interface_t* bootloader_get_uart_transport_interface(void) {
    return &blocking_uart_transport;
}