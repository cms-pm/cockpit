#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Platform Test Interface for UART Validation
 * 
 * Provides platform-specific hardware validation without depending on
 * the runtime abstractions being tested. Each platform implements this
 * interface using authoritative hardware access methods.
 * 
 * Design Philosophy:
 * - Tests validate abstractions, they don't depend on them
 * - Use vendor HAL structures as single source of truth
 * - Platform-specific validation, shared test logic
 * - Direct hardware access for embedded validation accuracy
 */
typedef struct {
    // Basic UART state validation
    bool (*uart_is_enabled)(void);
    bool (*uart_transmitter_enabled)(void);
    bool (*uart_receiver_enabled)(void);
    bool (*uart_tx_ready)(void);
    bool (*uart_tx_complete)(void);
    
    // Configuration validation
    uint32_t (*uart_get_configured_baud)(void);
    uint32_t (*uart_get_prescaler_value)(void);
    
    // Error and status validation
    bool (*uart_check_error_flags)(void);
    uint32_t (*uart_get_status_register)(void);
    
    // Future expansion points for other peripherals
    // bool (*gpio_is_output_mode)(uint8_t pin);
    // bool (*spi_is_enabled)(void);
    // etc.
} uart_test_interface_t;

// Platform interface access (injected at build time by workspace builder)
extern const uart_test_interface_t* platform_uart_test;

#ifdef __cplusplus
}
#endif