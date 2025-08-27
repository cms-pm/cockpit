/*
 * CockpitVM Bootloader - Modular Diagnostics Framework
 * Phase 4.6.3: Structured logging with timestamps, levels, and modular output drivers
 * 
 * Features:
 * - Timestamped structured logging
 * - Module/file/line tracking  
 * - Status code integration
 * - Modular output drivers (USART2 initial, expandable)
 * - Zero interference with Oracle protocol (USART1)
 */

#ifndef BOOTLOADER_DIAGNOSTICS_H
#define BOOTLOADER_DIAGNOSTICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
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
// Status Codes (bootloader-specific)
// =================================================================
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_GENERAL = 1,
    STATUS_ERROR_NANOPB = 2,
    STATUS_ERROR_FRAME = 3,
    STATUS_ERROR_PROTOCOL = 4,
    STATUS_ERROR_FLASH = 5,
    STATUS_ERROR_MEMORY = 6,
    STATUS_ERROR_TIMEOUT = 7,
    STATUS_ERROR_CRC = 8,
    STATUS_ERROR_STATE = 9
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
bool bootloader_diag_init(const diag_output_driver_t* driver, uint32_t baud_rate);

/**
 * Core logging function with full context
 * @param level Log level
 * @param module Module name (e.g., "PROTOCOL", "NANOPB")
 * @param file Source file name (__FILE__)
 * @param line Source line number (__LINE__)
 * @param status Status code
 * @param format Printf-style format string
 * @param ... Arguments for format string
 */
void bootloader_diag_log_full(log_level_t level, const char* module, const char* file, 
                             int line, status_code_t status, const char* format, ...);

/**
 * Protocol flow step logging (successor to protocol_flow_log_step)
 * @param step Single character step identifier (A-Z, 0-9)
 * @param description Step description
 * @param status Status code for this step
 */
void bootloader_diag_flow_step(char step, const char* description, status_code_t status);

/**
 * Binary data hex dump
 * @param label Description of data
 * @param data Binary data to dump
 * @param length Number of bytes
 */
void bootloader_diag_hex_dump(const char* label, const uint8_t* data, uint16_t length);

/**
 * nanopb encode/decode test with diagnostics
 */
void bootloader_diag_nanopb_test(void);

// =================================================================
// Convenience Macros
// =================================================================

// Core logging macros with automatic file/line/module context
#define DIAG_LOG(level, module, status, fmt, ...) \
    bootloader_diag_log_full(level, module, __FILE__, __LINE__, status, fmt, ##__VA_ARGS__)

// Level-specific DIAG macros with enhanced context
#define DIAG_ERROR(comp, fmt, ...)   DIAG_LOG(LOG_LEVEL_ERROR, comp, STATUS_ERROR_GENERAL, fmt, ##__VA_ARGS__)
#define DIAG_WARN(comp, fmt, ...)    DIAG_LOG(LOG_LEVEL_WARN, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)  
#define DIAG_INFO(comp, fmt, ...)    DIAG_LOG(LOG_LEVEL_INFO, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)
#define DIAG_DEBUG(comp, fmt, ...)   DIAG_LOG(LOG_LEVEL_DEBUG, comp, STATUS_SUCCESS, fmt, ##__VA_ARGS__)

// Enhanced DIAG macros with explicit status codes
#define DIAG_ERRORF(comp, status, fmt, ...)  DIAG_LOG(LOG_LEVEL_ERROR, comp, status, fmt, ##__VA_ARGS__)
#define DIAG_DEBUGF(comp, status, fmt, ...) DIAG_LOG(LOG_LEVEL_DEBUG, comp, status, fmt, ##__VA_ARGS__)

// Simplified legacy compatibility (for rapid migration)  
#define DIAG_ERRORF_LEGACY(comp, fmt, ...) DIAG_LOG(LOG_LEVEL_ERROR, comp, STATUS_ERROR_GENERAL, fmt, ##__VA_ARGS__)

// Quick compatibility overrides for existing protocol engine code
#undef DIAG_ERRORF
#define DIAG_ERRORF(comp, fmt, ...) DIAG_LOG(LOG_LEVEL_ERROR, comp, STATUS_ERROR_GENERAL, fmt, ##__VA_ARGS__)

#undef DIAG_FLOW  
#define DIAG_FLOW(step, desc) bootloader_diag_flow_step(step, desc, STATUS_SUCCESS)

// Common module names and component identifiers
#define MOD_PROTOCOL    "PROTOCOL"
#define MOD_NANOPB      "NANOPB"
#define MOD_FRAME       "FRAME"
#define MOD_FLASH       "FLASH"
#define MOD_MEMORY      "MEMORY"
#define MOD_GENERAL     "GENERAL"

// Legacy component compatibility (for existing DIAG usage)
#define DIAG_COMPONENT_PROTOCOL_ENGINE      MOD_PROTOCOL
#define DIAG_COMPONENT_NANOPB_DECODE        MOD_NANOPB
#define DIAG_COMPONENT_NANOPB_ENCODE        MOD_NANOPB
#define DIAG_COMPONENT_MESSAGE_HANDLER      MOD_PROTOCOL
#define DIAG_COMPONENT_FRAME_PARSER         MOD_FRAME

// Flow identifiers (A-J protocol steps)
#define DIAG_FLOW_A_FRAME_START                 'A'
#define DIAG_FLOW_B_FRAME_LENGTH                'B'
#define DIAG_FLOW_C_FRAME_PAYLOAD               'C'
#define DIAG_FLOW_D_FRAME_CRC_OK                'D'
#define DIAG_FLOW_E_PROTOBUF_DECODE_START       'E'
#define DIAG_FLOW_F_PROTOBUF_DECODE_OK          'F'
#define DIAG_FLOW_G_MESSAGE_PROCESSING          'G'
#define DIAG_FLOW_H_RESPONSE_GENERATION         'H'
#define DIAG_FLOW_I_RESPONSE_ENCODE_OK          'I'
#define DIAG_FLOW_J_RESPONSE_TRANSMITTED        'J'

// Flow step macros (spiritual successor to protocol_flow_log_step) - will be overridden below

// Binary buffer inspection macro
#define DIAG_BUFFER(level, comp, label, data, len) bootloader_diag_hex_dump(label, data, len)

// =================================================================
// Built-in Output Drivers
// =================================================================

// USART2 output driver (PA2/PA3)
extern const diag_output_driver_t diag_driver_usart2;

// Future drivers (for expansion)
// extern const diag_output_driver_t diag_driver_usart3;
// extern const diag_output_driver_t diag_driver_semihosting;
// extern const diag_output_driver_t diag_driver_file;

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_DIAGNOSTICS_H