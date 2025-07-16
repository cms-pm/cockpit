#include "bootloader_errors.h"
#include <string.h>

error_manager_t g_error_manager = {0};
volatile bootloader_state_t g_current_state = STATE_STARTUP;
static uint32_t g_system_tick = 0;

uint32_t get_system_tick(void) {
    return ++g_system_tick;
}

bootloader_state_t error_code_to_state(bootloader_error_code_t error_code) {
    switch (error_code) {
        case ERROR_UART_TIMEOUT:
        case ERROR_UART_FRAMING:
        case ERROR_UART_OVERRUN:
        case ERROR_UART_NOISE:
        case ERROR_UART_PARITY:
            return STATE_ERROR_COMMUNICATION;
            
        case ERROR_FLASH_ERASE_FAILED:
        case ERROR_FLASH_WRITE_FAILED:
        case ERROR_FLASH_VERIFY_FAILED:
        case ERROR_FLASH_LOCKED:
        case ERROR_FLASH_ALIGNMENT:
            return STATE_ERROR_FLASH_OPERATION;
            
        case ERROR_CRC_MISMATCH:
        case ERROR_INVALID_HEADER:
        case ERROR_INVALID_SIZE:
        case ERROR_INVALID_MAGIC:
        case ERROR_INVALID_VERSION:
            return STATE_ERROR_DATA_CORRUPTION;
            
        case ERROR_BUFFER_OVERFLOW:
        case ERROR_BUFFER_UNDERFLOW:
        case ERROR_MEMORY_EXHAUSTED:
        case ERROR_RESOURCE_LOCKED:
            return STATE_ERROR_RESOURCE_EXHAUSTION;
            
        case ERROR_OPERATION_TIMEOUT:
        case ERROR_RESPONSE_TIMEOUT:
        case ERROR_HANDSHAKE_TIMEOUT:
        case ERROR_TRANSFER_TIMEOUT:
            return STATE_ERROR_TIMEOUT;
            
        case ERROR_HARDWARE_FAULT:
        case ERROR_CLOCK_FAILURE:
        case ERROR_POWER_FAULT:
        case ERROR_PERIPHERAL_FAULT:
            return STATE_ERROR_HARDWARE_FAULT;
            
        default:
            return STATE_ERROR_HARDWARE_FAULT;
    }
}

const char* error_code_to_string(bootloader_error_code_t error_code) {
    switch (error_code) {
        case ERROR_NONE: return "NO_ERROR";
        
        // Communication errors
        case ERROR_UART_TIMEOUT: return "UART_TIMEOUT";
        case ERROR_UART_FRAMING: return "UART_FRAMING";
        case ERROR_UART_OVERRUN: return "UART_OVERRUN";
        case ERROR_UART_NOISE: return "UART_NOISE";
        case ERROR_UART_PARITY: return "UART_PARITY";
        
        // Flash operation errors
        case ERROR_FLASH_ERASE_FAILED: return "FLASH_ERASE_FAILED";
        case ERROR_FLASH_WRITE_FAILED: return "FLASH_WRITE_FAILED";
        case ERROR_FLASH_VERIFY_FAILED: return "FLASH_VERIFY_FAILED";
        case ERROR_FLASH_LOCKED: return "FLASH_LOCKED";
        case ERROR_FLASH_ALIGNMENT: return "FLASH_ALIGNMENT";
        
        // Data corruption errors
        case ERROR_CRC_MISMATCH: return "CRC_MISMATCH";
        case ERROR_INVALID_HEADER: return "INVALID_HEADER";
        case ERROR_INVALID_SIZE: return "INVALID_SIZE";
        case ERROR_INVALID_MAGIC: return "INVALID_MAGIC";
        case ERROR_INVALID_VERSION: return "INVALID_VERSION";
        
        // Resource errors
        case ERROR_BUFFER_OVERFLOW: return "BUFFER_OVERFLOW";
        case ERROR_BUFFER_UNDERFLOW: return "BUFFER_UNDERFLOW";
        case ERROR_MEMORY_EXHAUSTED: return "MEMORY_EXHAUSTED";
        case ERROR_RESOURCE_LOCKED: return "RESOURCE_LOCKED";
        
        // Protocol errors
        case ERROR_INVALID_COMMAND: return "INVALID_COMMAND";
        case ERROR_SEQUENCE_ERROR: return "SEQUENCE_ERROR";
        case ERROR_STATE_VIOLATION: return "STATE_VIOLATION";
        case ERROR_PROTOCOL_VERSION: return "PROTOCOL_VERSION";
        
        // Hardware errors
        case ERROR_HARDWARE_FAULT: return "HARDWARE_FAULT";
        case ERROR_CLOCK_FAILURE: return "CLOCK_FAILURE";
        case ERROR_POWER_FAULT: return "POWER_FAULT";
        case ERROR_PERIPHERAL_FAULT: return "PERIPHERAL_FAULT";
        
        // Timeout errors
        case ERROR_OPERATION_TIMEOUT: return "OPERATION_TIMEOUT";
        case ERROR_RESPONSE_TIMEOUT: return "RESPONSE_TIMEOUT";
        case ERROR_HANDSHAKE_TIMEOUT: return "HANDSHAKE_TIMEOUT";
        case ERROR_TRANSFER_TIMEOUT: return "TRANSFER_TIMEOUT";
        
        default: return "UNKNOWN_ERROR";
    }
}

const char* error_severity_to_string(error_severity_t severity) {
    switch (severity) {
        case ERROR_SEVERITY_INFO: return "INFO";
        case ERROR_SEVERITY_WARNING: return "WARNING";
        case ERROR_SEVERITY_ERROR: return "ERROR";
        case ERROR_SEVERITY_CRITICAL: return "CRITICAL";
        case ERROR_SEVERITY_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

const char* bootloader_state_to_string(bootloader_state_t state) {
    switch (state) {
        case STATE_STARTUP: return "STARTUP";
        case STATE_TRIGGER_DETECT: return "TRIGGER_DETECT";
        case STATE_BOOTLOADER_ACTIVE: return "BOOTLOADER_ACTIVE";
        case STATE_TRANSPORT_INIT: return "TRANSPORT_INIT";
        case STATE_HANDSHAKE: return "HANDSHAKE";
        case STATE_READY: return "READY";
        case STATE_RECEIVE_HEADER: return "RECEIVE_HEADER";
        case STATE_RECEIVE_DATA: return "RECEIVE_DATA";
        case STATE_VERIFY: return "VERIFY";
        case STATE_PROGRAM: return "PROGRAM";
        case STATE_BANK_SWITCH: return "BANK_SWITCH";
        case STATE_COMPLETE: return "COMPLETE";
        case STATE_ERROR_COMMUNICATION: return "ERROR_COMMUNICATION";
        case STATE_ERROR_FLASH_OPERATION: return "ERROR_FLASH_OPERATION";
        case STATE_ERROR_DATA_CORRUPTION: return "ERROR_DATA_CORRUPTION";
        case STATE_ERROR_RESOURCE_EXHAUSTION: return "ERROR_RESOURCE_EXHAUSTION";
        case STATE_ERROR_TIMEOUT: return "ERROR_TIMEOUT";
        case STATE_ERROR_HARDWARE_FAULT: return "ERROR_HARDWARE_FAULT";
        case STATE_RECOVERY_RETRY: return "RECOVERY_RETRY";
        case STATE_RECOVERY_ABORT: return "RECOVERY_ABORT";
        case STATE_JUMP_APPLICATION: return "JUMP_APPLICATION";
        default: return "UNKNOWN_STATE";
    }
}

void error_manager_init(error_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(error_manager_t));
}

void error_manager_log(error_manager_t* manager, const error_context_t* error) {
    if (!manager || !error) return;
    
    memcpy(&manager->errors[manager->error_index], error, sizeof(error_context_t));
    
    manager->error_index = (manager->error_index + 1) % MAX_ERROR_HISTORY;
    
    if (manager->error_count < MAX_ERROR_HISTORY) {
        manager->error_count++;
    }
    
    manager->total_error_count++;
    manager->last_error_timestamp = error->timestamp;
    
    if (error->severity >= ERROR_SEVERITY_CRITICAL) {
        manager->critical_error_count++;
    }
}

bool error_manager_get_last_error(error_manager_t* manager, error_context_t* error) {
    if (!manager || !error || manager->error_count == 0) {
        return false;
    }
    
    uint8_t last_index = (manager->error_index + MAX_ERROR_HISTORY - 1) % MAX_ERROR_HISTORY;
    memcpy(error, &manager->errors[last_index], sizeof(error_context_t));
    
    return true;
}

uint32_t error_manager_get_error_count(error_manager_t* manager, error_severity_t min_severity) {
    if (!manager) return 0;
    
    uint32_t count = 0;
    uint8_t entries_to_check = manager->error_count;
    
    for (uint8_t i = 0; i < entries_to_check; i++) {
        uint8_t index = (manager->error_index + MAX_ERROR_HISTORY - 1 - i) % MAX_ERROR_HISTORY;
        if (manager->errors[index].severity >= min_severity) {
            count++;
        }
    }
    
    return count;
}

void error_manager_clear_history(error_manager_t* manager) {
    if (!manager) return;
    
    memset(manager->errors, 0, sizeof(manager->errors));
    manager->error_count = 0;
    manager->error_index = 0;
}

bool error_manager_has_critical_errors(error_manager_t* manager) {
    if (!manager) return false;
    
    return manager->critical_error_count > 0;
}