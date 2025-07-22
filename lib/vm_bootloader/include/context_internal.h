/*
 * CockpitVM Bootloader - Internal Context Structure
 * 
 * Internal context structure definition for the unified bootloader.
 * This header is not part of the public API.
 */

#ifndef VM_BOOTLOADER_CONTEXT_INTERNAL_H
#define VM_BOOTLOADER_CONTEXT_INTERNAL_H

#include "vm_bootloader.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for managers (will be implemented in later chunks)
typedef struct vm_bootloader_protocol_context vm_bootloader_protocol_context_t;
typedef struct vm_bootloader_resource_manager vm_bootloader_resource_manager_t;
typedef struct vm_bootloader_error_manager vm_bootloader_error_manager_t;
typedef struct vm_bootloader_timeout_manager vm_bootloader_timeout_manager_t;

// Internal context structure
typedef struct vm_bootloader_context {
    // Lifecycle state
    bool initialized;                         // Initialization complete flag
    bool emergency_mode;                      // Emergency shutdown flag
    bool session_active;                      // Session active flag
    
    // State machine
    vm_bootloader_state_t current_state;     // Current bootloader state
    vm_bootloader_mode_t mode;               // Operating mode
    
    // Configuration
    uint32_t session_timeout_ms;             // Session timeout in milliseconds
    uint32_t frame_timeout_ms;               // Frame timeout in milliseconds
    bool enable_debug_output;                // Debug output flag
    bool enable_resource_tracking;          // Resource tracking flag
    bool enable_emergency_recovery;         // Emergency recovery flag
    
    // Timing
    uint32_t boot_time_ms;                   // Boot time timestamp
    uint32_t session_start_ms;               // Session start timestamp
    uint32_t last_activity_ms;               // Last activity timestamp
    
    // Statistics
    uint32_t execution_cycles;               // Main loop execution cycles
    uint32_t total_frames_received;          // Total frames received
    uint32_t total_frames_sent;              // Total frames sent
    uint32_t total_errors;                   // Total error count
    uint32_t successful_operations;          // Successful operations count
    
    // Version and build information
    const char* version_string;              // Version string
    const char* build_timestamp;             // Build timestamp
    
    // Subsystem contexts (will be implemented in later chunks)
    vm_bootloader_protocol_context_t* protocol_ctx;    // Protocol engine context
    vm_bootloader_resource_manager_t* resource_mgr;    // Resource manager
    vm_bootloader_error_manager_t* error_mgr;         // Error manager
    vm_bootloader_timeout_manager_t* timeout_mgr;     // Timeout manager
    
} vm_bootloader_context_internal_t;

// Ensure the public context type is compatible with internal structure
_Static_assert(sizeof(vm_bootloader_context_t) >= sizeof(vm_bootloader_context_internal_t),
               "Public context size must accommodate internal structure");

#ifdef __cplusplus
}
#endif

#endif // VM_BOOTLOADER_CONTEXT_INTERNAL_H