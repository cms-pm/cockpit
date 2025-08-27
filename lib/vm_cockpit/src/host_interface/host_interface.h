/*
 * VM Cockpit Host Interface
 * Embedded Native API for Hardware Abstraction
 * 
 * Provides clean, professional embedded API that bridges VM to host hardware.
 * Uses embedded-native naming conventions for scalability and clarity.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// System Initialization
// =================================================================

/**
 * @brief Initialize host interface and underlying platform
 * Must be called before any other host interface functions
 */
void host_interface_init(void);

// =================================================================
// GPIO Operations
// =================================================================

typedef enum {
    GPIO_INPUT = 0,
    GPIO_OUTPUT,
    GPIO_INPUT_PULLUP,
    GPIO_INPUT_PULLDOWN
} gpio_mode_t;

/**
 * @brief Configure GPIO pin mode
 * @param pin Pin number (0-based)
 * @param mode Pin configuration mode
 */
void gpio_pin_config(uint8_t pin, gpio_mode_t mode);

/**
 * @brief Write digital value to GPIO pin
 * @param pin Pin number (0-based)
 * @param state true = HIGH, false = LOW
 */
void gpio_pin_write(uint8_t pin, bool state);

/**
 * @brief Read digital value from GPIO pin
 * @param pin Pin number (0-based)
 * @return true = HIGH, false = LOW
 */
bool gpio_pin_read(uint8_t pin);

// =================================================================
// UART Operations
// =================================================================

/**
 * @brief Initialize UART with specified baud rate
 * @param baud_rate Baud rate (e.g. 115200)
 */
void uart_begin(uint32_t baud_rate);

/**
 * @brief Write null-terminated string to UART
 * @param str String to transmit
 */
void uart_write_string(const char* str);

/**
 * @brief Write single character to UART
 * @param c Character to transmit
 */
void uart_write_char(char c);

/**
 * @brief Write binary frame data to UART atomically
 * @param frame_data Pointer to frame data buffer
 * @param frame_length Length of frame data in bytes
 */
void uart_write_frame(const uint8_t* frame_data, uint16_t frame_length);

/**
 * @brief Check if UART data is available for reading
 * @return true if data available, false otherwise
 */
bool uart_data_available(void);

/**
 * @brief Read single character from UART
 * @return Received character, or 0 if no data available
 */
char uart_read_char(void);

// =================================================================
// Debug UART Operations (USART2 - Oracle-clean diagnostics)
// =================================================================

/**
 * @brief Initialize debug UART with specified baud rate
 * @param baud_rate Debug UART baud rate (e.g. 115200)
 */
void debug_uart_begin(uint32_t baud_rate);

/**
 * @brief Write null-terminated string to debug UART
 * @param str String to transmit
 */
void debug_uart_write_string(const char* str);

/**
 * @brief Write binary data to debug UART
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 */
void debug_uart_write_data(const uint8_t* data, uint16_t length);

// =================================================================
// Timing Operations
// =================================================================

/**
 * @brief Blocking delay in milliseconds
 * @param milliseconds Delay duration
 */
void delay_ms(uint32_t milliseconds);

/**
 * @brief Blocking delay in microseconds
 * @param microseconds Delay duration
 */
void delay_us(uint32_t microseconds);

/**
 * @brief Get system tick count in milliseconds
 * @return Milliseconds since system start
 */
uint32_t get_tick_ms(void);

/**
 * @brief Get system tick count in microseconds
 * @return Microseconds since system start
 */
uint32_t get_tick_us(void);

// =================================================================
// Future Expansion: Analog Operations
// =================================================================

/**
 * @brief Initialize ADC for specified pin
 * @param pin Pin number for ADC input
 */
void adc_init(uint8_t pin);

/**
 * @brief Read analog value from ADC pin
 * @param pin Pin number to read
 * @return ADC value (0-4095 for 12-bit ADC)
 */
uint16_t adc_read(uint8_t pin);

/**
 * @brief Write PWM value to pin
 * @param pin Pin number for PWM output
 * @param value PWM value (0-1023)
 */
void pwm_write(uint8_t pin, uint16_t value);

#ifdef __cplusplus
}
#endif