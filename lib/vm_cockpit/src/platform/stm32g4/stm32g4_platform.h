/*
 * STM32G4 Platform Interface
 * STM32 HAL-First Platform Adapter for VM Cockpit
 * 
 * Provides clean interface to STM32G431CB hardware using STM32 HAL exclusively.
 * This layer owns all hardware initialization and register access.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)
#include "stm32g4xx_hal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// Platform Initialization
// =================================================================

/**
 * @brief Initialize STM32G4 platform
 * Configures clocks, HAL, and basic peripherals using STM32 HAL
 */
void stm32g4_platform_init(void);

/**
 * @brief Configure system clock using STM32 HAL
 * Sets up HSE, PLL, and system clock to 160MHz
 */
void SystemClock_Config(void);

/**
 * @brief Error handler (defined in application)
 */
extern void Error_Handler(void);

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// =================================================================
// GPIO Platform Interface
// =================================================================

/**
 * @brief Configure GPIO pin
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin GPIO pin number (0-15)
 * @param mode GPIO mode (INPUT, OUTPUT, etc.)
 * @param pull GPIO pull resistor configuration
 */
void stm32g4_gpio_config(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t pull);

/**
 * @brief Write digital value to GPIO pin
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin GPIO pin mask (GPIO_PIN_0, GPIO_PIN_1, etc.)
 * @param state GPIO_PIN_SET or GPIO_PIN_RESET
 */
void stm32g4_gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);

/**
 * @brief Read digital value from GPIO pin
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin GPIO pin mask (GPIO_PIN_0, GPIO_PIN_1, etc.)
 * @return GPIO_PIN_SET or GPIO_PIN_RESET
 */
GPIO_PinState stm32g4_gpio_read(GPIO_TypeDef* port, uint16_t pin);

// =================================================================
// UART Platform Interface
// =================================================================

/**
 * @brief Initialize UART1 with specified baud rate
 * @param baud_rate Baud rate for UART communication
 * @return HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef stm32g4_uart_init(uint32_t baud_rate);

/**
 * @brief Transmit data via UART
 * @param data Pointer to data buffer
 * @param size Number of bytes to transmit
 * @return HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef stm32g4_uart_transmit(uint8_t* data, uint16_t size);

/**
 * @brief Check if UART data is available
 * @return true if data available, false otherwise
 */
bool stm32g4_uart_data_available(void);

/**
 * @brief Receive single byte from UART
 * @param data Pointer to store received byte
 * @return HAL_OK on success, HAL_ERROR/HAL_TIMEOUT on failure
 */
HAL_StatusTypeDef stm32g4_uart_receive(uint8_t* data);

#endif // PLATFORM_STM32G4 && !QEMU_PLATFORM

// =================================================================
// Timing Platform Interface
// =================================================================

/**
 * @brief Blocking delay in milliseconds
 * @param milliseconds Delay duration
 */
void stm32g4_delay_ms(uint32_t milliseconds);

/**
 * @brief Get system tick count in milliseconds
 * @return Milliseconds since system start
 */
uint32_t stm32g4_get_tick_ms(void);

// =================================================================
// Pin Mapping Configuration
// =================================================================

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Pin mapping for WeAct Studio STM32G431CB board
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin_mask;
    uint8_t pin_number;
} stm32g4_pin_mapping_t;

/**
 * @brief Get GPIO port and pin for logical pin number
 * @param logical_pin Logical pin number (0-based)
 * @return Pointer to pin mapping structure, NULL if invalid
 */
const stm32g4_pin_mapping_t* stm32g4_get_pin_mapping(uint8_t logical_pin);

#endif // PLATFORM_STM32G4 && !QEMU_PLATFORM

#ifdef __cplusplus
}
#endif