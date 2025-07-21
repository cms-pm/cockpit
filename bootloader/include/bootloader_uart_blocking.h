/*
 * Bootloader Blocking UART Transport Header
 * 
 * Simple, reliable blocking UART operations for bootloader foundation.
 * Designed for deterministic execution and easy debugging.
 */

#ifndef BOOTLOADER_UART_BLOCKING_H
#define BOOTLOADER_UART_BLOCKING_H

#include <stdint.h>
#include <stdbool.h>
#include "transport_interface.h"
#include "bootloader_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

// Bootloader UART configuration
#define BOOTLOADER_UART_BAUD_RATE           115200
#define BOOTLOADER_UART_POLL_DELAY_US       100    // Small delay to prevent CPU burning

// Blocking UART context
typedef struct {
    uint32_t baud_rate;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t error_count;
    uint32_t timeout_count;
    bool initialized;
} blocking_uart_context_t;

// Public API Functions
bootloader_error_t bootloader_uart_init(void);
bootloader_error_t bootloader_uart_send_bytes(const uint8_t* data, uint16_t len, uint32_t timeout_ms);
bootloader_error_t bootloader_uart_receive_bytes(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms);
bool bootloader_uart_data_available(void);

// Transport interface access
const transport_interface_t* bootloader_get_uart_transport_interface(void);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_UART_BLOCKING_H