/*
 * QEMU Platform Implementation
 * Virtual hardware platform adapter for QEMU LM3S6965EVB
 * 
 * This platform layer provides QEMU-specific implementations using semihosting
 * as the equivalent of STM32 HAL for virtual hardware control.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../platform_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// QEMU Platform Initialization
// =================================================================

/**
 * @brief Initialize QEMU platform
 * Sets up semihosting and virtual hardware interfaces
 */
void qemu_platform_init(void);

// =================================================================
// QEMU-Specific Functions
// =================================================================

/**
 * @brief QEMU virtual GPIO simulation
 * Uses internal state tracking since QEMU doesn't have real GPIO
 */
void qemu_gpio_simulate_state(uint8_t pin, bool state);

/**
 * @brief Get QEMU GPIO simulation state
 * @param pin Logical pin number
 * @return Current simulated state
 */
bool qemu_gpio_get_simulated_state(uint8_t pin);

/**
 * @brief QEMU virtual UART via semihosting
 * @param data Data to output
 * @param size Data size
 */
void qemu_uart_semihost_output(const uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif