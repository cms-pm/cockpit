/*
 * QEMU Semihosting Interface
 * QEMU's HAL Equivalent - Virtual Hardware Control via Semihosting
 * 
 * This provides the Layer 3 equivalent for QEMU - just as STM32 HAL provides
 * register-level abstractions for real hardware, semihosting provides 
 * virtual hardware abstractions for QEMU simulation.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// Semihosting System Calls (QEMU's "Hardware" Interface)
// =================================================================

// ARM semihosting operation numbers
#define SYS_WRITEC          0x03    // Write character
#define SYS_WRITE0          0x04    // Write null-terminated string
#define SYS_WRITE           0x05    // Write buffer
#define SYS_READ            0x06    // Read buffer
#define SYS_READC           0x07    // Read character
#define SYS_CLOCK           0x10    // Get system clock
#define SYS_TIME            0x11    // Get time
#define SYS_EXIT            0x18    // Exit simulation

/**
 * @brief Perform semihosting call
 * This is the fundamental interface to QEMU's virtual hardware
 * @param operation Semihosting operation number
 * @param parameter Parameter block pointer
 * @return Operation result
 */
uint32_t semihost_call(uint32_t operation, void* parameter);

// =================================================================
// QEMU Virtual Hardware Abstractions (HAL Equivalent)
// =================================================================

/**
 * @brief Initialize semihosting interface
 * Equivalent to HAL_Init() for STM32
 */
void qemu_semihost_init(void);

/**
 * @brief Write character via semihosting
 * @param c Character to write
 */
void qemu_semihost_putchar(char c);

/**
 * @brief Write string via semihosting
 * @param str Null-terminated string
 */
void qemu_semihost_puts(const char* str);

/**
 * @brief Write buffer via semihosting
 * @param data Data buffer
 * @param size Number of bytes
 */
void qemu_semihost_write(const uint8_t* data, uint32_t size);

/**
 * @brief Read character via semihosting (if available)
 * @return Character read, or -1 if none available
 */
int qemu_semihost_getchar(void);

/**
 * @brief Get system time via semihosting
 * @return System time in milliseconds
 */
uint32_t qemu_semihost_get_time_ms(void);

/**
 * @brief Simple delay implementation for QEMU
 * @param ms Milliseconds to delay
 * Note: In QEMU this is approximate since it's software simulation
 */
void qemu_semihost_delay_ms(uint32_t ms);

/**
 * @brief Exit QEMU simulation
 * @param exit_code Exit code for simulation
 */
void qemu_semihost_exit(uint32_t exit_code);

// =================================================================
// QEMU Virtual GPIO (Simulated Hardware)
// =================================================================

#define QEMU_MAX_GPIO_PINS 32

/**
 * @brief Initialize virtual GPIO system
 */
void qemu_gpio_init(void);

/**
 * @brief Set virtual GPIO pin state
 * @param pin Pin number
 * @param state Pin state (true = high, false = low)
 */
void qemu_gpio_set_pin(uint8_t pin, bool state);

/**
 * @brief Get virtual GPIO pin state
 * @param pin Pin number
 * @return Pin state (true = high, false = low)
 */
bool qemu_gpio_get_pin(uint8_t pin);

/**
 * @brief Set virtual GPIO pin direction
 * @param pin Pin number
 * @param output true for output, false for input
 */
void qemu_gpio_set_direction(uint8_t pin, bool output);

#ifdef __cplusplus
}
#endif