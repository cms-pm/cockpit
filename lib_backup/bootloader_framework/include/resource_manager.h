/*
 * ComponentVM Bootloader Framework - Resource Manager
 * 
 * Provides systematic resource tracking and cleanup to prevent
 * memory leaks, hardware lockups, and resource exhaustion in 
 * the bootloader. Critical for production embedded systems.
 * 
 * This addresses the gap in the canonical protocol implementation
 * which has no resource management capabilities.
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of resources that can be tracked
#define RESOURCE_MANAGER_MAX_RESOURCES 16

// Resource types specific to bootloader operations
typedef enum {
    RESOURCE_TYPE_NONE = 0,
    RESOURCE_TYPE_UART,              // UART peripheral configuration
    RESOURCE_TYPE_FLASH,             // Flash operation context  
    RESOURCE_TYPE_PROTOCOL_BUFFER,   // Protocol frame buffers
    RESOURCE_TYPE_STAGING_BUFFER,    // Flash staging buffers
    RESOURCE_TYPE_TIMEOUT_CONTEXT,   // Timeout tracking context
    RESOURCE_TYPE_ERROR_CONTEXT,     // Error diagnostic context
    RESOURCE_TYPE_SESSION_CONTEXT,   // Protocol session context
    RESOURCE_TYPE_CUSTOM             // Application-specific resource
} resource_type_t;

// Resource state tracking
typedef enum {
    RESOURCE_STATE_UNINITIALIZED = 0,
    RESOURCE_STATE_INITIALIZED,
    RESOURCE_STATE_IN_USE,
    RESOURCE_STATE_ERROR,
    RESOURCE_STATE_CLEANUP_PENDING,
    RESOURCE_STATE_CLEANED_UP
} resource_state_t;

// Resource priority for cleanup ordering
typedef enum {
    RESOURCE_PRIORITY_LOW = 0,       // Clean up last (buffers, contexts)
    RESOURCE_PRIORITY_MEDIUM,        // Clean up middle (protocol state)
    RESOURCE_PRIORITY_HIGH,          // Clean up early (hardware resources)
    RESOURCE_PRIORITY_CRITICAL       // Clean up first (safety-critical resources)
} resource_priority_t;

// Resource cleanup function signature
typedef void (*resource_cleanup_fn_t)(void* resource_handle);

// Resource diagnostic function signature  
typedef const char* (*resource_diagnostic_fn_t)(void* resource_handle);

// Individual resource entry
typedef struct {
    resource_type_t type;
    resource_state_t state;
    resource_priority_t priority;
    
    void* resource_handle;           // Pointer to actual resource
    resource_cleanup_fn_t cleanup_fn; // Function to clean up resource
    resource_diagnostic_fn_t diagnostic_fn; // Optional diagnostic function
    
    const char* resource_name;       // Human-readable name for debugging
    const char* file_registered;     // __FILE__ where resource was registered
    uint32_t line_registered;        // __LINE__ where resource was registered
    uint32_t timestamp_registered;   // When resource was registered
    
    bool auto_cleanup;               // Auto-cleanup on error
    bool critical_resource;          // Critical for system operation
    bool cleanup_on_emergency;       // Clean up during emergency shutdown
} resource_entry_t;

// Resource manager context
typedef struct resource_manager {
    resource_entry_t resources[RESOURCE_MANAGER_MAX_RESOURCES];
    uint8_t resource_count;
    bool initialized;
    bool emergency_mode;
    
    // Statistics
    uint32_t total_resources_registered;
    uint32_t total_resources_cleaned;
    uint32_t total_cleanup_failures;
    uint32_t emergency_cleanups;
} resource_manager_t;

// Resource manager initialization result
typedef enum {
    RESOURCE_MANAGER_SUCCESS = 0,
    RESOURCE_MANAGER_ERROR_ALREADY_INITIALIZED,
    RESOURCE_MANAGER_ERROR_NULL_POINTER,
    RESOURCE_MANAGER_ERROR_INVALID_CONFIG
} resource_manager_result_t;

// === RESOURCE MANAGER LIFECYCLE ===

/**
 * Initialize the resource manager
 */
resource_manager_result_t resource_manager_init(resource_manager_t* mgr);

/**
 * Cleanup the resource manager and all registered resources
 */
void resource_manager_cleanup_all(resource_manager_t* mgr);

/**
 * Emergency cleanup - force cleanup of all critical resources
 */
void resource_manager_emergency_cleanup(resource_manager_t* mgr);

// === RESOURCE REGISTRATION ===

/**
 * Register a resource for tracking and automatic cleanup
 */
resource_manager_result_t resource_manager_register(
    resource_manager_t* mgr,
    resource_type_t type,
    void* resource_handle,
    resource_cleanup_fn_t cleanup_fn,
    const char* resource_name,
    const char* file,
    uint32_t line
);

/**
 * Register a resource with full options
 */
resource_manager_result_t resource_manager_register_full(
    resource_manager_t* mgr,
    resource_type_t type,
    void* resource_handle,
    resource_cleanup_fn_t cleanup_fn,
    resource_diagnostic_fn_t diagnostic_fn,
    const char* resource_name,
    resource_priority_t priority,
    bool auto_cleanup,
    bool critical_resource,
    const char* file,
    uint32_t line
);

/**
 * Unregister a resource (resource is no longer tracked)
 */
resource_manager_result_t resource_manager_unregister(
    resource_manager_t* mgr,
    void* resource_handle
);

// === RESOURCE CLEANUP ===

/**
 * Clean up resources of a specific type
 */
resource_manager_result_t resource_manager_cleanup_by_type(
    resource_manager_t* mgr,
    resource_type_t type
);

/**
 * Clean up resources of a specific priority level
 */
resource_manager_result_t resource_manager_cleanup_by_priority(
    resource_manager_t* mgr,
    resource_priority_t priority
);

/**
 * Clean up a specific resource
 */
resource_manager_result_t resource_manager_cleanup_resource(
    resource_manager_t* mgr,
    void* resource_handle
);

// === RESOURCE QUERIES ===

/**
 * Check if a specific resource is registered
 */
bool resource_manager_is_registered(
    const resource_manager_t* mgr,
    void* resource_handle
);

/**
 * Get resource count by type
 */
uint8_t resource_manager_get_count_by_type(
    const resource_manager_t* mgr,
    resource_type_t type
);

/**
 * Get total resource count
 */
uint8_t resource_manager_get_total_count(const resource_manager_t* mgr);

/**
 * Check if resource manager has capacity for more resources
 */
bool resource_manager_has_capacity(const resource_manager_t* mgr);

// === DIAGNOSTICS ===

/**
 * Get resource manager statistics
 */
typedef struct {
    uint8_t active_resources;
    uint8_t total_registered;
    uint32_t total_cleaned;
    uint32_t cleanup_failures;
    uint32_t emergency_cleanups;
    bool emergency_mode;
} resource_manager_stats_t;

void resource_manager_get_stats(
    const resource_manager_t* mgr,
    resource_manager_stats_t* stats
);

/**
 * Print resource manager diagnostic information
 * (Only available in debug builds)
 */
void resource_manager_print_diagnostics(const resource_manager_t* mgr);

/**
 * Validate resource manager integrity
 * Returns true if all resources are in valid states
 */
bool resource_manager_validate(const resource_manager_t* mgr);

// === CONVENIENCE MACROS ===

// Macro for registering resources with automatic file/line tracking
#define RESOURCE_REGISTER(mgr, type, handle, cleanup_fn, name) \
    resource_manager_register(mgr, type, handle, cleanup_fn, name, __FILE__, __LINE__)

#define RESOURCE_REGISTER_FULL(mgr, type, handle, cleanup_fn, diag_fn, name, priority, auto_cleanup, critical) \
    resource_manager_register_full(mgr, type, handle, cleanup_fn, diag_fn, name, priority, auto_cleanup, critical, __FILE__, __LINE__)

// === STANDARD RESOURCE CLEANUP FUNCTIONS ===

/**
 * Standard cleanup functions for common bootloader resources
 * These can be used directly with resource_manager_register()
 */
void resource_cleanup_uart(void* uart_handle);
void resource_cleanup_flash_context(void* flash_context);
void resource_cleanup_protocol_buffer(void* buffer);
void resource_cleanup_staging_buffer(void* staging_buffer);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_MANAGER_H