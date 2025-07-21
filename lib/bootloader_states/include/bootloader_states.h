/*
 * Bootloader State Definitions
 * 
 * Single source of truth for bootloader state machine states.
 * Used by both implementation and test code.
 */

#ifndef BOOTLOADER_STATES_H
#define BOOTLOADER_STATES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bootloader state machine states - single source of truth
typedef enum {
    // Operational states
    BOOTLOADER_STATE_INIT = 0,
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
    BOOTLOADER_STATE_RECOVERY_ABORT,
    
    // Total count
    BOOTLOADER_STATE_COUNT
} bootloader_state_t;

// State information structure
typedef struct {
    bootloader_state_t state;
    const char* name;
    bool is_error_state;
    bool allows_retry;
} bootloader_state_info_t;

// State information table
extern const bootloader_state_info_t bootloader_state_table[BOOTLOADER_STATE_COUNT];

// State utility functions
const char* bootloader_get_state_name(bootloader_state_t state);
bool bootloader_is_error_state(bootloader_state_t state);
bool bootloader_state_allows_retry(bootloader_state_t state);
bool bootloader_is_valid_transition(bootloader_state_t from, bootloader_state_t to);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_STATES_H