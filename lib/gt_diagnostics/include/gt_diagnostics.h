/*
 * Golden Triangle Diagnostics Framework
 * Phase 4.8: DIAG Integration for Golden Triangle Tests
 * 
 * Extracted subset from vm_bootloader/bootloader_diagnostics.h
 * Features:
 * - Timestamped structured logging via USART2
 * - Module/file/line tracking  
 * - Status code integration
 * - Zero interference with Oracle protocol (USART1)
 * - Mutual exclusion with bootloader runtime console
 */

#ifndef GT_DIAGNOSTICS_H
#define GT_DIAGNOSTICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Prevent simultaneous operation with bootloader runtime console
#ifdef BOOTLOADER_RUNTIME_CONSOLE_ENABLED
    #error "Golden Triangle DIAG and Bootloader Runtime Console are mutually exclusive"
#endif

// =================================================================
// Log Levels
// =================================================================
typedef enum {
    LOG_LEVEL_ERROR = 0,    // Critical errors
    LOG_LEVEL_WARN  = 1,    // Warnings
    LOG_LEVEL_INFO  = 2,    // General information
    LOG_LEVEL_DEBUG = 3,    // Debug information
    LOG_LEVEL_TRACE = 4     // Detailed tracing
} log_level_t;

// =================================================================
// Status Codes (Golden Triangle specific)
// =================================================================
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_GENERAL = 1,
    STATUS_ERROR_I2C = 2,
    STATUS_ERROR_UART = 3,
    STATUS_ERROR_GPIO = 4,
    STATUS_ERROR_TIMEOUT = 5,
    STATUS_ERROR_HARDWARE = 6,
    STATUS_ERROR_TEST = 7
} status_code_t;

// =================================================================
// Output Driver Interface
// =================================================================
typedef struct {
    const char* name;
    bool (*init)(uint32_t baud_rate);
    bool (*write)(const char* message, uint16_t length);
    void (*flush)(void);
} diag_output_driver_t;

// =================================================================
// Core Diagnostics API
// =================================================================

/**
 * Initialize diagnostics system with output driver
 * @param driver Output driver (NULL for default USART2)
 * @param baud_rate Baud rate for UART drivers
 * @return true on success
 */
bool gt_diag_init(const diag_output_driver_t* driver, uint32_t baud_rate);

/**
 * Core logging function with full context
 * @param level Log level
 * @param module Module name (e.g., "I2C_TEST", "UART_TEST")
 * @param file Source file name (__FILE__)
 * @param line Source line number (__LINE__)
 * @param status Status code
 * @param format Printf-style format string
 * @param ... Arguments for format string
 */
void gt_diag_log_full(log_level_t level, const char* module, const char* file, 
                      int line, status_code_t status, const char* format, ...);

/**
 * Test flow step logging
 * @param step Single character step identifier (A-Z, 0-9)
 * @param description Step description
 * @param status Status code for this step
 */
void gt_diag_flow_step(char step, const char* description, status_code_t status);

/**
 * Binary data hex dump
 * @param label Description of data
 * @param data Binary data to dump
 * @param length Number of bytes
 */
void gt_diag_hex_dump(const char* label, const uint8_t* data, uint16_t length);

// =================================================================
// Golden Triangle Convenience Macros
// =================================================================

// Core logging macros with automatic file/line/module context
#define GT_DIAG_LOG(level, module, status, fmt, ...) \
    gt_diag_log_full(level, module, __FILE__, __LINE__, status, fmt, ##__VA_ARGS__)

// Level-specific DIAG macros
#define GT_DIAG_ERROR(comp, fmt, ...)   GT_DIAG_LOG(LOG_LEVEL_ERROR, comp, STATUS_ERROR_GENERAL, fmt, ##__VA_ARGS__)
#define GT_DIAG_WARN(comp, fmt, ...)    GT_DIAG_LOG(LOG_LEVEL_WARN, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)  
#define GT_DIAG_INFO(comp, fmt, ...)    GT_DIAG_LOG(LOG_LEVEL_INFO, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)
#define GT_DIAG_DEBUG(comp, fmt, ...)   GT_DIAG_LOG(LOG_LEVEL_DEBUG, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)

// Enhanced DIAG macros with explicit status codes
#define GT_DIAG_ERROR_STATUS(comp, status, fmt, ...)  GT_DIAG_LOG(LOG_LEVEL_ERROR, comp, status, fmt, ##__VA_ARGS__)

// Flow step macros
#define GT_DIAG_FLOW(step, desc) gt_diag_flow_step(step, desc, STATUS_SUCCESS)

// Binary buffer inspection macro
#define GT_DIAG_BUFFER(label, data, len) gt_diag_hex_dump(label, data, len)

// Common Golden Triangle module names
#define GT_MOD_I2C_TEST     "I2C_TEST"
#define GT_MOD_UART_TEST    "UART_TEST"
#define GT_MOD_GPIO_TEST    "GPIO_TEST"
#define GT_MOD_SPI_TEST     "SPI_TEST"
#define GT_MOD_ADC_TEST     "ADC_TEST"
#define GT_MOD_PWM_TEST     "PWM_TEST"
#define GT_MOD_TIMER_TEST   "TIMER_TEST"
#define GT_MOD_GENERAL      "GENERAL"

// =================================================================
// Built-in Output Drivers
// =================================================================

// USART2 output driver (PA2/PA3) - Golden Triangle default
extern const diag_output_driver_t gt_diag_driver_usart2;

#ifdef __cplusplus
}
#endif

#endif // GT_DIAGNOSTICS_H