/*
 * VM Cockpit Platform Interface
 * Common interface for all platform implementations
 * 
 * This header defines the common interface that all platform adapters must implement.
 * Each platform (STM32G4, QEMU, etc.) provides its own implementation of these functions.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// Platform Abstraction Types
// =================================================================

// Common platform result codes
typedef enum {
    PLATFORM_OK = 0,
    PLATFORM_ERROR,
    PLATFORM_TIMEOUT,
    PLATFORM_INVALID_PARAM,
    PLATFORM_NOT_SUPPORTED
} platform_result_t;

// GPIO states - platform independent
typedef enum {
    PLATFORM_GPIO_LOW = 0,
    PLATFORM_GPIO_HIGH = 1
} platform_gpio_state_t;

// GPIO modes - platform independent  
typedef enum {
    PLATFORM_GPIO_INPUT = 0,
    PLATFORM_GPIO_OUTPUT,
    PLATFORM_GPIO_INPUT_PULLUP,
    PLATFORM_GPIO_INPUT_PULLDOWN
} platform_gpio_mode_t;

// =================================================================
// Common Platform Interface (implemented by each platform)
// =================================================================

/**
 * @brief Initialize the platform
 * Each platform implements this to set up clocks, HAL, etc.
 */
void platform_init(void);

/**
 * @brief Platform-specific delay in milliseconds
 * @param milliseconds Delay duration
 */
void platform_delay_ms(uint32_t milliseconds);

/**
 * @brief Get platform tick count in milliseconds
 * @return Milliseconds since platform start
 */
uint32_t platform_get_tick_ms(void);

/**
 * @brief Configure GPIO pin
 * @param logical_pin Platform-independent pin number
 * @param mode GPIO mode configuration
 * @return Platform result code
 */
platform_result_t platform_gpio_config(uint8_t logical_pin, platform_gpio_mode_t mode);

/**
 * @brief Write to GPIO pin
 * @param logical_pin Platform-independent pin number
 * @param state GPIO state to write
 * @return Platform result code
 */
platform_result_t platform_gpio_write(uint8_t logical_pin, platform_gpio_state_t state);

/**
 * @brief Read from GPIO pin
 * @param logical_pin Platform-independent pin number
 * @param state Pointer to store read state
 * @return Platform result code
 */
platform_result_t platform_gpio_read(uint8_t logical_pin, platform_gpio_state_t* state);

/**
 * @brief Initialize UART with specified baud rate
 * @param baud_rate UART baud rate
 * @return Platform result code
 */
platform_result_t platform_uart_init(uint32_t baud_rate);

/**
 * @brief Transmit data via UART
 * @param data Pointer to data buffer
 * @param size Number of bytes to transmit
 * @return Platform result code
 */
platform_result_t platform_uart_transmit(const uint8_t* data, uint16_t size);

/**
 * @brief Check if UART data is available
 * @return true if data available, false otherwise
 */
bool platform_uart_data_available(void);

/**
 * @brief Receive single byte from UART
 * @param data Pointer to store received byte
 * @return Platform result code
 */
platform_result_t platform_uart_receive(uint8_t* data);

#ifdef __cplusplus
}
#endif