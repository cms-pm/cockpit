/*
 * Bootloader Simplified State Machine for Blocking Operations
 * 
 * Simple, deterministic state machine optimized for blocking operations.
 * Uses simplified timeout and resource management.
 */

#ifndef BOOTLOADER_STATE_BLOCKING_H
#define BOOTLOADER_STATE_BLOCKING_H

#include "bootloader_errors.h"
#include "bootloader_timeout.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simple state machine states
typedef enum {
    // Operational states
    BOOTLOADER_STATE_INIT,
    BOOTLOADER_STATE_IDLE,
    BOOTLOADER_STATE_HANDSHAKE,
    BOOTLOADER_STATE_READY,
    BOOTLOADER_STATE_RECEIVE_DATA,
    BOOTLOADER_STATE_VERIFY,
    BOOTLOADER_STATE_PROGRAM,
    BOOTLOADER_STATE_COMPLETE,
    
    // Hierarchical error states (per QA plan)
    BOOTLOADER_STATE_ERROR_COMMUNICATION,     // UART timeouts, framing errors
    BOOTLOADER_STATE_ERROR_FLASH_OPERATION,   // Flash erase/write failures  
    BOOTLOADER_STATE_ERROR_DATA_CORRUPTION,   // CRC mismatches
    BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION, // Buffer overflows
    
    // Recovery states
    BOOTLOADER_STATE_RECOVERY_RETRY,
    BOOTLOADER_STATE_RECOVERY_ABORT
} bootloader_state_blocking_t;

// Error context for diagnostics
typedef struct {
    bootloader_error_code_t error_code;
    bootloader_state_blocking_t previous_state;
    char diagnostic_info[64];
    uint32_t error_timestamp;
    uint8_t retry_count;
} error_context_blocking_t;

// Simple state machine context
typedef struct {
    bootloader_state_blocking_t current_state;
    bootloader_state_blocking_t previous_state;
    uint32_t state_entry_time;
    uint32_t state_transition_count;
    error_context_blocking_t error_context;
    bool initialized;
} bootloader_state_machine_blocking_t;

// State machine functions
void bootloader_state_machine_init_blocking(bootloader_state_machine_blocking_t* sm);
bootloader_error_t bootloader_run_cycle_blocking(bootloader_state_machine_blocking_t* sm);
void bootloader_transition_to_state_blocking(bootloader_state_machine_blocking_t* sm, bootloader_state_blocking_t new_state);
void bootloader_transition_to_error_blocking(bootloader_state_machine_blocking_t* sm, bootloader_error_code_t error_code, const char* diagnostic);

// State query functions
bootloader_state_blocking_t bootloader_get_current_state_blocking(const bootloader_state_machine_blocking_t* sm);
bool bootloader_is_error_state_blocking(const bootloader_state_machine_blocking_t* sm);
bool bootloader_can_retry_blocking(const bootloader_state_machine_blocking_t* sm);
const char* bootloader_get_state_name_blocking(bootloader_state_blocking_t state);

// State handlers (blocking implementations)
bootloader_error_t handle_handshake_blocking(bootloader_state_machine_blocking_t* sm);
bootloader_error_t handle_data_reception_blocking(bootloader_state_machine_blocking_t* sm);
bootloader_error_t handle_verification_blocking(bootloader_state_machine_blocking_t* sm);
bootloader_error_t handle_flash_programming_blocking(bootloader_state_machine_blocking_t* sm);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_STATE_BLOCKING_H