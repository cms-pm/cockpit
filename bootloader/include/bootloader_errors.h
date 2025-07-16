#ifndef BOOTLOADER_ERRORS_H
#define BOOTLOADER_ERRORS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // Operational states
    STATE_STARTUP,
    STATE_TRIGGER_DETECT,
    STATE_BOOTLOADER_ACTIVE,
    STATE_TRANSPORT_INIT,
    STATE_HANDSHAKE,
    STATE_READY,
    STATE_RECEIVE_HEADER,
    STATE_RECEIVE_DATA,
    STATE_VERIFY,
    STATE_PROGRAM,
    STATE_BANK_SWITCH,
    STATE_COMPLETE,
    
    // Context-aware error states
    STATE_ERROR_COMMUNICATION,     // UART timeout, framing errors
    STATE_ERROR_FLASH_OPERATION,   // Flash erase/write failures  
    STATE_ERROR_DATA_CORRUPTION,   // CRC mismatches, invalid data
    STATE_ERROR_RESOURCE_EXHAUSTION, // Memory/buffer issues
    STATE_ERROR_TIMEOUT,           // Generic timeout errors
    STATE_ERROR_HARDWARE_FAULT,    // Hardware-specific failures
    
    // Recovery states
    STATE_RECOVERY_RETRY,
    STATE_RECOVERY_ABORT,
    
    STATE_JUMP_APPLICATION
} bootloader_state_t;

typedef enum {
    ERROR_NONE = 0,
    
    // Communication errors
    ERROR_UART_TIMEOUT,
    ERROR_UART_FRAMING,
    ERROR_UART_OVERRUN,
    ERROR_UART_NOISE,
    ERROR_UART_PARITY,
    
    // Flash operation errors
    ERROR_FLASH_ERASE_FAILED,
    ERROR_FLASH_WRITE_FAILED,
    ERROR_FLASH_VERIFY_FAILED,
    ERROR_FLASH_LOCKED,
    ERROR_FLASH_ALIGNMENT,
    
    // Data corruption errors
    ERROR_CRC_MISMATCH,
    ERROR_INVALID_HEADER,
    ERROR_INVALID_SIZE,
    ERROR_INVALID_MAGIC,
    ERROR_INVALID_VERSION,
    
    // Resource errors
    ERROR_BUFFER_OVERFLOW,
    ERROR_BUFFER_UNDERFLOW,
    ERROR_MEMORY_EXHAUSTED,
    ERROR_RESOURCE_LOCKED,
    
    // Protocol errors
    ERROR_INVALID_COMMAND,
    ERROR_SEQUENCE_ERROR,
    ERROR_STATE_VIOLATION,
    ERROR_PROTOCOL_VERSION,
    
    // Hardware errors
    ERROR_HARDWARE_FAULT,
    ERROR_CLOCK_FAILURE,
    ERROR_POWER_FAULT,
    ERROR_PERIPHERAL_FAULT,
    
    // Timeout errors
    ERROR_OPERATION_TIMEOUT,
    ERROR_RESPONSE_TIMEOUT,
    ERROR_HANDSHAKE_TIMEOUT,
    ERROR_TRANSFER_TIMEOUT
} bootloader_error_code_t;

typedef enum {
    ERROR_SEVERITY_INFO = 0,
    ERROR_SEVERITY_WARNING,
    ERROR_SEVERITY_ERROR,
    ERROR_SEVERITY_CRITICAL,
    ERROR_SEVERITY_FATAL
} error_severity_t;

typedef struct {
    bootloader_error_code_t error_code;
    error_severity_t severity;
    bootloader_state_t source_state;
    uint32_t timestamp;
    uint32_t line_number;
    const char* file_name;
    const char* function_name;
    uint32_t context_data;
    char description[64];
} error_context_t;

#define MAX_ERROR_HISTORY 16

typedef struct {
    error_context_t errors[MAX_ERROR_HISTORY];
    uint8_t error_count;
    uint8_t error_index;
    uint32_t total_error_count;
    uint32_t critical_error_count;
    uint32_t last_error_timestamp;
} error_manager_t;

#define ERROR_CONTEXT(code, severity, data, desc) \
    { \
        .error_code = (code), \
        .severity = (severity), \
        .source_state = g_current_state, \
        .timestamp = get_system_tick(), \
        .line_number = __LINE__, \
        .file_name = __FILE__, \
        .function_name = __func__, \
        .context_data = (data), \
        .description = (desc) \
    }

#define LOG_ERROR(code, severity, data, desc) \
    error_manager_log(&g_error_manager, ERROR_CONTEXT(code, severity, data, desc))

#define LOG_COMMUNICATION_ERROR(code, data) \
    LOG_ERROR(code, ERROR_SEVERITY_ERROR, data, "Communication failure")

#define LOG_FLASH_ERROR(code, data) \
    LOG_ERROR(code, ERROR_SEVERITY_CRITICAL, data, "Flash operation failure")

#define LOG_DATA_ERROR(code, data) \
    LOG_ERROR(code, ERROR_SEVERITY_ERROR, data, "Data corruption detected")

#define LOG_TIMEOUT_ERROR(code, data) \
    LOG_ERROR(code, ERROR_SEVERITY_WARNING, data, "Operation timeout")

#define LOG_HARDWARE_ERROR(code, data) \
    LOG_ERROR(code, ERROR_SEVERITY_CRITICAL, data, "Hardware fault")

bootloader_state_t error_code_to_state(bootloader_error_code_t error_code);
const char* error_code_to_string(bootloader_error_code_t error_code);
const char* error_severity_to_string(error_severity_t severity);
const char* bootloader_state_to_string(bootloader_state_t state);

void error_manager_init(error_manager_t* manager);
void error_manager_log(error_manager_t* manager, const error_context_t* error);
bool error_manager_get_last_error(error_manager_t* manager, error_context_t* error);
uint32_t error_manager_get_error_count(error_manager_t* manager, error_severity_t min_severity);
void error_manager_clear_history(error_manager_t* manager);
bool error_manager_has_critical_errors(error_manager_t* manager);

extern error_manager_t g_error_manager;
extern volatile bootloader_state_t g_current_state;

uint32_t get_system_tick(void);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_ERRORS_H