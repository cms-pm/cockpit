/*
 * Bootloader Simplified State Machine Implementation
 * 
 * Simple, deterministic state machine for blocking operations.
 * Easy to debug with linear execution flow.
 */

#include "bootloader_state_blocking.h"
#include "bootloader_uart_blocking.h"
#include "bootloader_timeout.h"
#include "host_interface/host_interface.h"
#include <string.h>
#include <stdio.h>

// Global state machine instance
static bootloader_state_machine_blocking_t g_state_machine = {0};

void bootloader_state_machine_init_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm) return;
    
    memset(sm, 0, sizeof(bootloader_state_machine_blocking_t));
    sm->current_state = BOOTLOADER_STATE_INIT;
    sm->previous_state = BOOTLOADER_STATE_INIT;
    sm->state_entry_time = get_tick_ms();
    sm->initialized = true;
}

bootloader_error_t bootloader_run_cycle_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm || !sm->initialized) {
        return BOOTLOADER_ERROR_INVALID_PARAM;
    }
    
    bootloader_error_t result = BOOTLOADER_SUCCESS;
    
    switch (sm->current_state) {
        case BOOTLOADER_STATE_INIT:
            // Initialize bootloader components
            result = bootloader_uart_init();
            if (result == BOOTLOADER_SUCCESS) {
                bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_IDLE);
            } else {
                bootloader_transition_to_error_blocking(sm, BOOTLOADER_ERROR_UART_INIT, "UART initialization failed");
            }
            break;
            
        case BOOTLOADER_STATE_IDLE:
            // Wait for handshake to begin bootloader session
            bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_HANDSHAKE);
            break;
            
        case BOOTLOADER_STATE_HANDSHAKE:
            result = handle_handshake_blocking(sm);
            break;
            
        case BOOTLOADER_STATE_READY:
            // Ready to receive data - transition to receive state
            bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_RECEIVE_DATA);
            break;
            
        case BOOTLOADER_STATE_RECEIVE_DATA:
            result = handle_data_reception_blocking(sm);
            break;
            
        case BOOTLOADER_STATE_VERIFY:
            result = handle_verification_blocking(sm);
            break;
            
        case BOOTLOADER_STATE_PROGRAM:
            result = handle_flash_programming_blocking(sm);
            break;
            
        case BOOTLOADER_STATE_COMPLETE:
            // Bootloader session complete - could transition to application
            break;
            
        // Error states
        case BOOTLOADER_STATE_ERROR_COMMUNICATION:
        case BOOTLOADER_STATE_ERROR_FLASH_OPERATION:
        case BOOTLOADER_STATE_ERROR_DATA_CORRUPTION:
        case BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION:
            // Handle error recovery if possible
            if (bootloader_can_retry_blocking(sm)) {
                bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_RECOVERY_RETRY);
            } else {
                bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_RECOVERY_ABORT);
            }
            break;
            
        case BOOTLOADER_STATE_RECOVERY_RETRY:
            // Attempt to retry the failed operation
            sm->error_context.retry_count++;
            bootloader_transition_to_state_blocking(sm, sm->error_context.previous_state);
            break;
            
        case BOOTLOADER_STATE_RECOVERY_ABORT:
            // Graceful abort - cleanup and exit
            break;
            
        default:
            bootloader_transition_to_error_blocking(sm, BOOTLOADER_ERROR_INVALID_STATE, "Unknown state");
            result = BOOTLOADER_ERROR_INVALID_STATE;
            break;
    }
    
    return result;
}

void bootloader_transition_to_state_blocking(bootloader_state_machine_blocking_t* sm, bootloader_state_blocking_t new_state) {
    if (!sm) return;
    
    sm->previous_state = sm->current_state;
    sm->current_state = new_state;
    sm->state_entry_time = get_tick_ms();
    sm->state_transition_count++;
}

void bootloader_transition_to_error_blocking(bootloader_state_machine_blocking_t* sm, bootloader_error_code_t error_code, const char* diagnostic) {
    if (!sm) return;
    
    // Preserve error context
    sm->error_context.error_code = error_code;
    sm->error_context.previous_state = sm->current_state;
    sm->error_context.error_timestamp = get_tick_ms();
    
    if (diagnostic) {
        strncpy(sm->error_context.diagnostic_info, diagnostic, sizeof(sm->error_context.diagnostic_info) - 1);
        sm->error_context.diagnostic_info[sizeof(sm->error_context.diagnostic_info) - 1] = '\0';
    }
    
    // Transition to appropriate error state based on error code
    bootloader_state_blocking_t error_state;
    switch (error_code) {
        case BOOTLOADER_ERROR_UART_TIMEOUT:
        case BOOTLOADER_ERROR_UART_HARDWARE:
            error_state = BOOTLOADER_STATE_ERROR_COMMUNICATION;
            break;
            
        case BOOTLOADER_ERROR_FLASH_ERASE:
        case BOOTLOADER_ERROR_FLASH_PROGRAM:
            error_state = BOOTLOADER_STATE_ERROR_FLASH_OPERATION;
            break;
            
        case BOOTLOADER_ERROR_CRC_MISMATCH:
        case BOOTLOADER_ERROR_INVALID_DATA:
            error_state = BOOTLOADER_STATE_ERROR_DATA_CORRUPTION;
            break;
            
        case BOOTLOADER_ERROR_BUFFER_OVERFLOW:
        case BOOTLOADER_ERROR_OUT_OF_MEMORY:
            error_state = BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION;
            break;
            
        default:
            error_state = BOOTLOADER_STATE_ERROR_COMMUNICATION;
            break;
    }
    
    bootloader_transition_to_state_blocking(sm, error_state);
}

bootloader_state_blocking_t bootloader_get_current_state_blocking(const bootloader_state_machine_blocking_t* sm) {
    if (!sm) return BOOTLOADER_STATE_INIT;
    return sm->current_state;
}

bool bootloader_is_error_state_blocking(const bootloader_state_machine_blocking_t* sm) {
    if (!sm) return false;
    
    return (sm->current_state >= BOOTLOADER_STATE_ERROR_COMMUNICATION && 
            sm->current_state <= BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION);
}

bool bootloader_can_retry_blocking(const bootloader_state_machine_blocking_t* sm) {
    if (!sm) return false;
    
    const uint8_t MAX_RETRIES = 3;  // Balanced retry strategy
    return sm->error_context.retry_count < MAX_RETRIES;
}

const char* bootloader_get_state_name_blocking(bootloader_state_blocking_t state) {
    switch (state) {
        case BOOTLOADER_STATE_INIT: return "INIT";
        case BOOTLOADER_STATE_IDLE: return "IDLE";
        case BOOTLOADER_STATE_HANDSHAKE: return "HANDSHAKE";
        case BOOTLOADER_STATE_READY: return "READY";
        case BOOTLOADER_STATE_RECEIVE_DATA: return "RECEIVE_DATA";
        case BOOTLOADER_STATE_VERIFY: return "VERIFY";
        case BOOTLOADER_STATE_PROGRAM: return "PROGRAM";
        case BOOTLOADER_STATE_COMPLETE: return "COMPLETE";
        case BOOTLOADER_STATE_ERROR_COMMUNICATION: return "ERROR_COMMUNICATION";
        case BOOTLOADER_STATE_ERROR_FLASH_OPERATION: return "ERROR_FLASH_OPERATION";
        case BOOTLOADER_STATE_ERROR_DATA_CORRUPTION: return "ERROR_DATA_CORRUPTION";
        case BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION: return "ERROR_RESOURCE_EXHAUSTION";
        case BOOTLOADER_STATE_RECOVERY_RETRY: return "RECOVERY_RETRY";
        case BOOTLOADER_STATE_RECOVERY_ABORT: return "RECOVERY_ABORT";
        default: return "UNKNOWN";
    }
}

// State handler implementations
bootloader_error_t handle_handshake_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm) return BOOTLOADER_ERROR_INVALID_PARAM;
    
    // Simple handshake: expect magic bytes 0x55, 0xAA, 0x01, 0x02
    uint8_t expected_handshake[] = {0x55, 0xAA, 0x01, 0x02};
    uint8_t received_handshake[4];
    uint16_t bytes_received;
    
    // Blocking receive with timeout
    bootloader_error_t result = bootloader_uart_receive_bytes(
        received_handshake, 
        sizeof(received_handshake), 
        &bytes_received, 
        BOOTLOADER_HANDSHAKE_TIMEOUT_MS
    );
    
    if (result != BOOTLOADER_SUCCESS) {
        bootloader_transition_to_error_blocking(sm, result, "Handshake timeout");
        return result;
    }
    
    if (bytes_received != sizeof(expected_handshake)) {
        bootloader_transition_to_error_blocking(sm, BOOTLOADER_ERROR_INVALID_DATA, "Incomplete handshake");
        return BOOTLOADER_ERROR_INVALID_DATA;
    }
    
    // Validate handshake
    if (memcmp(received_handshake, expected_handshake, sizeof(expected_handshake)) != 0) {
        bootloader_transition_to_error_blocking(sm, BOOTLOADER_ERROR_INVALID_DATA, "Invalid handshake");
        return BOOTLOADER_ERROR_INVALID_DATA;
    }
    
    // Send handshake acknowledgment
    uint8_t ack[] = {0xAA, 0x55, 0x02, 0x01};
    result = bootloader_uart_send_bytes(ack, sizeof(ack), BOOTLOADER_CHUNK_TIMEOUT_MS);
    
    if (result != BOOTLOADER_SUCCESS) {
        bootloader_transition_to_error_blocking(sm, result, "Handshake ACK failed");
        return result;
    }
    
    // Handshake successful
    bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_READY);
    return BOOTLOADER_SUCCESS;
}

bootloader_error_t handle_data_reception_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm) return BOOTLOADER_ERROR_INVALID_PARAM;
    
    // Placeholder for data reception logic
    // For now, just transition to verify state
    bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_VERIFY);
    return BOOTLOADER_SUCCESS;
}

bootloader_error_t handle_verification_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm) return BOOTLOADER_ERROR_INVALID_PARAM;
    
    // Placeholder for verification logic
    // For now, just transition to program state
    bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_PROGRAM);
    return BOOTLOADER_SUCCESS;
}

bootloader_error_t handle_flash_programming_blocking(bootloader_state_machine_blocking_t* sm) {
    if (!sm) return BOOTLOADER_ERROR_INVALID_PARAM;
    
    // Placeholder for flash programming logic
    // For now, just transition to complete state
    bootloader_transition_to_state_blocking(sm, BOOTLOADER_STATE_COMPLETE);
    return BOOTLOADER_SUCCESS;
}

// Public API to access global state machine
bootloader_state_machine_blocking_t* bootloader_get_state_machine_blocking(void) {
    return &g_state_machine;
}