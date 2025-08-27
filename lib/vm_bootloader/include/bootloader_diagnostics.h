/*
 * CockpitVM Bootloader Diagnostics
 * 
 * Oracle-style diagnostic logging system for bootloader debugging.
 * Outputs to USART2 (PA2/PA3) to avoid Oracle protocol interference on USART1.
 */

#ifndef BOOTLOADER_DIAGNOSTICS_H
#define BOOTLOADER_DIAGNOSTICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Diagnostic levels
typedef enum {
    DIAG_LEVEL_ERROR = 0,
    DIAG_LEVEL_WARN,
    DIAG_LEVEL_INFO,
    DIAG_LEVEL_DEBUG,
    DIAG_LEVEL_TRACE
} bootloader_diag_level_t;

// Component identifiers for structured logging
typedef enum {
    DIAG_COMPONENT_PROTOCOL_ENGINE = 0,
    DIAG_COMPONENT_FRAME_PARSER,
    DIAG_COMPONENT_NANOPB_DECODE,
    DIAG_COMPONENT_NANOPB_ENCODE,
    DIAG_COMPONENT_MESSAGE_HANDLER,
    DIAG_COMPONENT_FLASH_PROGRAMMER,
    DIAG_COMPONENT_VM_BOOTLOADER,
    DIAG_COMPONENT_CONTEXT_MANAGER
} bootloader_diag_component_t;

// A-J Flow step identifiers
typedef enum {
    DIAG_FLOW_A_FRAME_START = 'A',
    DIAG_FLOW_B_FRAME_LENGTH = 'B', 
    DIAG_FLOW_C_FRAME_PAYLOAD = 'C',
    DIAG_FLOW_D_FRAME_CRC_OK = 'D',
    DIAG_FLOW_E_PROTOBUF_DECODE_START = 'E',
    DIAG_FLOW_F_PROTOBUF_DECODE_OK = 'F',
    DIAG_FLOW_G_MESSAGE_PROCESSING = 'G',
    DIAG_FLOW_H_RESPONSE_GENERATION = 'H',
    DIAG_FLOW_I_RESPONSE_ENCODE_OK = 'I',
    DIAG_FLOW_J_RESPONSE_TRANSMITTED = 'J'
} bootloader_diag_flow_step_t;

// =================================================================
// Initialization and Configuration
// =================================================================

/**
 * @brief Initialize bootloader diagnostics system
 * @param enable_output Enable/disable diagnostic output
 * @return true on success, false on failure
 */
bool bootloader_diag_init(bool enable_output);

/**
 * @brief Set diagnostic output level
 * @param level Minimum level for output
 */
void bootloader_diag_set_level(bootloader_diag_level_t level);

/**
 * @brief Enable/disable diagnostic output
 * @param enabled Output enable state
 */
void bootloader_diag_set_enabled(bool enabled);

// =================================================================
// Oracle-Style Diagnostic Logging
// =================================================================

/**
 * @brief Log structured diagnostic message (Oracle style)
 * @param level Diagnostic level
 * @param component Component identifier
 * @param message Diagnostic message
 */
void bootloader_diag_log(bootloader_diag_level_t level, 
                        bootloader_diag_component_t component,
                        const char* message);

/**
 * @brief Log formatted diagnostic message (Oracle style)
 * @param level Diagnostic level
 * @param component Component identifier
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void bootloader_diag_logf(bootloader_diag_level_t level, 
                         bootloader_diag_component_t component,
                         const char* format, ...);

/**
 * @brief Log A-J protocol flow step with timestamp
 * @param flow_step A-J flow step identifier
 * @param context Additional context information
 */
void bootloader_diag_flow_step(bootloader_diag_flow_step_t flow_step, 
                              const char* context);

/**
 * @brief Log buffer contents in hex format
 * @param level Diagnostic level
 * @param component Component identifier
 * @param label Buffer description
 * @param buffer Buffer data
 * @param length Buffer length
 */
void bootloader_diag_log_buffer(bootloader_diag_level_t level,
                               bootloader_diag_component_t component,
                               const char* label,
                               const uint8_t* buffer,
                               size_t length);

// =================================================================
// Convenience Macros (Oracle Style)
// =================================================================

#define DIAG_ERROR(component, msg) \
    bootloader_diag_log(DIAG_LEVEL_ERROR, component, msg)

#define DIAG_WARN(component, msg) \
    bootloader_diag_log(DIAG_LEVEL_WARN, component, msg)

#define DIAG_INFO(component, msg) \
    bootloader_diag_log(DIAG_LEVEL_INFO, component, msg)

#define DIAG_DEBUG(component, msg) \
    bootloader_diag_log(DIAG_LEVEL_DEBUG, component, msg)

#define DIAG_TRACE(component, msg) \
    bootloader_diag_log(DIAG_LEVEL_TRACE, component, msg)

#define DIAG_ERRORF(component, fmt, ...) \
    bootloader_diag_logf(DIAG_LEVEL_ERROR, component, fmt, ##__VA_ARGS__)

#define DIAG_WARNF(component, fmt, ...) \
    bootloader_diag_logf(DIAG_LEVEL_WARN, component, fmt, ##__VA_ARGS__)

#define DIAG_INFOF(component, fmt, ...) \
    bootloader_diag_logf(DIAG_LEVEL_INFO, component, fmt, ##__VA_ARGS__)

#define DIAG_DEBUGF(component, fmt, ...) \
    bootloader_diag_logf(DIAG_LEVEL_DEBUG, component, fmt, ##__VA_ARGS__)

#define DIAG_TRACEF(component, fmt, ...) \
    bootloader_diag_logf(DIAG_LEVEL_TRACE, component, fmt, ##__VA_ARGS__)

#define DIAG_FLOW(step, context) \
    bootloader_diag_flow_step(step, context)

#define DIAG_BUFFER(level, component, label, buf, len) \
    bootloader_diag_log_buffer(level, component, label, buf, len)

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_DIAGNOSTICS_H