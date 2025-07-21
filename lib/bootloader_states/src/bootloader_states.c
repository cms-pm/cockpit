/*
 * Bootloader State Definitions Implementation
 * 
 * Implementation of state utility functions and state table.
 */

#include "bootloader_states.h"
#include <string.h>
#include <stdbool.h>

// State information table - single source of truth
const bootloader_state_info_t bootloader_state_table[BOOTLOADER_STATE_COUNT] = {
    // Operational states
    {BOOTLOADER_STATE_INIT, "INIT", false, false},
    {BOOTLOADER_STATE_IDLE, "IDLE", false, false},
    {BOOTLOADER_STATE_HANDSHAKE, "HANDSHAKE", false, true},
    {BOOTLOADER_STATE_READY, "READY", false, false},
    {BOOTLOADER_STATE_RECEIVE_DATA, "RECEIVE_DATA", false, true},
    {BOOTLOADER_STATE_VERIFY, "VERIFY", false, true},
    {BOOTLOADER_STATE_PROGRAM, "PROGRAM", false, true},
    {BOOTLOADER_STATE_COMPLETE, "COMPLETE", false, false},
    
    // Error states
    {BOOTLOADER_STATE_ERROR_COMMUNICATION, "ERROR_COMMUNICATION", true, true},
    {BOOTLOADER_STATE_ERROR_FLASH_OPERATION, "ERROR_FLASH_OPERATION", true, true},
    {BOOTLOADER_STATE_ERROR_DATA_CORRUPTION, "ERROR_DATA_CORRUPTION", true, false},
    {BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION, "ERROR_RESOURCE_EXHAUSTION", true, false},
    
    // Recovery states
    {BOOTLOADER_STATE_RECOVERY_RETRY, "RECOVERY_RETRY", false, false},
    {BOOTLOADER_STATE_RECOVERY_ABORT, "RECOVERY_ABORT", false, false}
};

const char* bootloader_get_state_name(bootloader_state_t state) {
    if (state >= BOOTLOADER_STATE_COUNT) {
        return "INVALID";
    }
    return bootloader_state_table[state].name;
}

bool bootloader_is_error_state(bootloader_state_t state) {
    if (state >= BOOTLOADER_STATE_COUNT) {
        return false;
    }
    return bootloader_state_table[state].is_error_state;
}

bool bootloader_state_allows_retry(bootloader_state_t state) {
    if (state >= BOOTLOADER_STATE_COUNT) {
        return false;
    }
    return bootloader_state_table[state].allows_retry;
}

bool bootloader_is_valid_transition(bootloader_state_t from, bootloader_state_t to) {
    // Validate state bounds
    if (from >= BOOTLOADER_STATE_COUNT || to >= BOOTLOADER_STATE_COUNT) {
        return false;
    }
    
    // Define valid state transition matrix
    // This is where we encode the actual state machine logic
    switch (from) {
        case BOOTLOADER_STATE_INIT:
            return (to == BOOTLOADER_STATE_IDLE);
            
        case BOOTLOADER_STATE_IDLE:
            return (to == BOOTLOADER_STATE_HANDSHAKE || 
                    bootloader_is_error_state(to));
            
        case BOOTLOADER_STATE_HANDSHAKE:
            return (to == BOOTLOADER_STATE_READY || 
                    to == BOOTLOADER_STATE_ERROR_COMMUNICATION);
            
        case BOOTLOADER_STATE_READY:
            return (to == BOOTLOADER_STATE_RECEIVE_DATA || 
                    bootloader_is_error_state(to));
            
        case BOOTLOADER_STATE_RECEIVE_DATA:
            return (to == BOOTLOADER_STATE_VERIFY || 
                    to == BOOTLOADER_STATE_ERROR_COMMUNICATION ||
                    to == BOOTLOADER_STATE_ERROR_DATA_CORRUPTION);
            
        case BOOTLOADER_STATE_VERIFY:
            return (to == BOOTLOADER_STATE_PROGRAM || 
                    to == BOOTLOADER_STATE_ERROR_DATA_CORRUPTION);
            
        case BOOTLOADER_STATE_PROGRAM:
            return (to == BOOTLOADER_STATE_COMPLETE || 
                    to == BOOTLOADER_STATE_ERROR_FLASH_OPERATION);
            
        case BOOTLOADER_STATE_COMPLETE:
            return (to == BOOTLOADER_STATE_IDLE);  // Can restart
            
        // Error states can transition to recovery
        case BOOTLOADER_STATE_ERROR_COMMUNICATION:
        case BOOTLOADER_STATE_ERROR_FLASH_OPERATION:
        case BOOTLOADER_STATE_ERROR_DATA_CORRUPTION:
        case BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION:
            return (to == BOOTLOADER_STATE_RECOVERY_RETRY || 
                    to == BOOTLOADER_STATE_RECOVERY_ABORT);
            
        // Recovery states
        case BOOTLOADER_STATE_RECOVERY_RETRY:
            // Can retry to previous operational state or abort
            return (!bootloader_is_error_state(to) || 
                    to == BOOTLOADER_STATE_RECOVERY_ABORT);
            
        case BOOTLOADER_STATE_RECOVERY_ABORT:
            return (to == BOOTLOADER_STATE_IDLE);  // Reset to idle
            
        default:
            return false;
    }
}