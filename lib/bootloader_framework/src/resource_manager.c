/*
 * ComponentVM Bootloader Framework - Resource Manager Implementation
 * 
 * Provides systematic resource tracking and cleanup to prevent
 * memory leaks, hardware lockups, and resource exhaustion.
 */

#include "resource_manager.h"
#include "host_interface/host_interface.h"

#include <string.h>
#include <stdio.h>

// === RESOURCE MANAGER LIFECYCLE ===

resource_manager_result_t resource_manager_init(resource_manager_t* mgr)
{
    if (!mgr) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    if (mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_ALREADY_INITIALIZED;
    }
    
    // Initialize resource manager state
    memset(mgr, 0, sizeof(resource_manager_t));
    mgr->initialized = true;
    mgr->emergency_mode = false;
    mgr->resource_count = 0;
    
    return RESOURCE_MANAGER_SUCCESS;
}

void resource_manager_cleanup_all(resource_manager_t* mgr)
{
    if (!mgr || !mgr->initialized) {
        return;
    }
    
    // Clean up resources in priority order (critical first)
    for (resource_priority_t priority = RESOURCE_PRIORITY_CRITICAL; 
         priority >= RESOURCE_PRIORITY_LOW; 
         priority--) {
        resource_manager_cleanup_by_priority(mgr, priority);
    }
    
    // Reset state
    mgr->resource_count = 0;
    mgr->total_resources_cleaned += mgr->resource_count;
}

void resource_manager_emergency_cleanup(resource_manager_t* mgr)
{
    if (!mgr || !mgr->initialized) {
        return;
    }
    
    mgr->emergency_mode = true;
    mgr->emergency_cleanups++;
    
    // Emergency cleanup - only critical resources that need immediate attention
    for (int i = 0; i < mgr->resource_count; i++) {
        resource_entry_t* entry = &mgr->resources[i];
        
        if (entry->cleanup_on_emergency && 
            entry->state != RESOURCE_STATE_CLEANED_UP &&
            entry->cleanup_fn) {
            
            entry->cleanup_fn(entry->resource_handle);
            entry->state = RESOURCE_STATE_CLEANED_UP;
        }
    }
}

// === RESOURCE REGISTRATION ===

resource_manager_result_t resource_manager_register(
    resource_manager_t* mgr,
    resource_type_t type,
    void* resource_handle,
    resource_cleanup_fn_t cleanup_fn,
    const char* resource_name,
    const char* file,
    uint32_t line)
{
    return resource_manager_register_full(
        mgr, type, resource_handle, cleanup_fn, NULL, resource_name,
        RESOURCE_PRIORITY_MEDIUM, true, false, file, line
    );
}

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
    uint32_t line)
{
    if (!mgr || !mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    if (mgr->resource_count >= RESOURCE_MANAGER_MAX_RESOURCES) {
        return RESOURCE_MANAGER_ERROR_INVALID_CONFIG; // Resource manager full
    }
    
    // Check if resource is already registered
    if (resource_manager_is_registered(mgr, resource_handle)) {
        return RESOURCE_MANAGER_ERROR_INVALID_CONFIG; // Already registered
    }
    
    // Add new resource entry
    resource_entry_t* entry = &mgr->resources[mgr->resource_count];
    entry->type = type;
    entry->state = RESOURCE_STATE_INITIALIZED;
    entry->priority = priority;
    entry->resource_handle = resource_handle;
    entry->cleanup_fn = cleanup_fn;
    entry->diagnostic_fn = diagnostic_fn;
    entry->resource_name = resource_name;
    entry->file_registered = file;
    entry->line_registered = line;
    entry->timestamp_registered = get_tick_ms();
    entry->auto_cleanup = auto_cleanup;
    entry->critical_resource = critical_resource;
    entry->cleanup_on_emergency = critical_resource; // Critical resources cleaned on emergency
    
    mgr->resource_count++;
    mgr->total_resources_registered++;
    
    return RESOURCE_MANAGER_SUCCESS;
}

resource_manager_result_t resource_manager_unregister(
    resource_manager_t* mgr,
    void* resource_handle)
{
    if (!mgr || !mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    // Find the resource
    for (int i = 0; i < mgr->resource_count; i++) {
        if (mgr->resources[i].resource_handle == resource_handle) {
            // Found the resource - remove it by shifting array
            for (int j = i; j < mgr->resource_count - 1; j++) {
                mgr->resources[j] = mgr->resources[j + 1];
            }
            mgr->resource_count--;
            return RESOURCE_MANAGER_SUCCESS;
        }
    }
    
    return RESOURCE_MANAGER_ERROR_INVALID_CONFIG; // Resource not found
}

// === RESOURCE CLEANUP ===

resource_manager_result_t resource_manager_cleanup_by_type(
    resource_manager_t* mgr,
    resource_type_t type)
{
    if (!mgr || !mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    for (int i = 0; i < mgr->resource_count; i++) {
        resource_entry_t* entry = &mgr->resources[i];
        
        if (entry->type == type && 
            entry->state != RESOURCE_STATE_CLEANED_UP &&
            entry->cleanup_fn) {
            
            entry->cleanup_fn(entry->resource_handle);
            entry->state = RESOURCE_STATE_CLEANED_UP;
            mgr->total_resources_cleaned++;
        }
    }
    
    return RESOURCE_MANAGER_SUCCESS;
}

resource_manager_result_t resource_manager_cleanup_by_priority(
    resource_manager_t* mgr,
    resource_priority_t priority)
{
    if (!mgr || !mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    for (int i = 0; i < mgr->resource_count; i++) {
        resource_entry_t* entry = &mgr->resources[i];
        
        if (entry->priority == priority && 
            entry->state != RESOURCE_STATE_CLEANED_UP &&
            entry->cleanup_fn) {
            
            entry->cleanup_fn(entry->resource_handle);
            entry->state = RESOURCE_STATE_CLEANED_UP;
            mgr->total_resources_cleaned++;
        }
    }
    
    return RESOURCE_MANAGER_SUCCESS;
}

resource_manager_result_t resource_manager_cleanup_resource(
    resource_manager_t* mgr,
    void* resource_handle)
{
    if (!mgr || !mgr->initialized) {
        return RESOURCE_MANAGER_ERROR_NULL_POINTER;
    }
    
    // Find and clean up specific resource
    for (int i = 0; i < mgr->resource_count; i++) {
        resource_entry_t* entry = &mgr->resources[i];
        
        if (entry->resource_handle == resource_handle &&
            entry->state != RESOURCE_STATE_CLEANED_UP &&
            entry->cleanup_fn) {
            
            entry->cleanup_fn(entry->resource_handle);
            entry->state = RESOURCE_STATE_CLEANED_UP;
            mgr->total_resources_cleaned++;
            return RESOURCE_MANAGER_SUCCESS;
        }
    }
    
    return RESOURCE_MANAGER_ERROR_INVALID_CONFIG; // Resource not found
}

// === RESOURCE QUERIES ===

bool resource_manager_is_registered(
    const resource_manager_t* mgr,
    void* resource_handle)
{
    if (!mgr || !mgr->initialized) {
        return false;
    }
    
    for (int i = 0; i < mgr->resource_count; i++) {
        if (mgr->resources[i].resource_handle == resource_handle) {
            return true;
        }
    }
    
    return false;
}

uint8_t resource_manager_get_count_by_type(
    const resource_manager_t* mgr,
    resource_type_t type)
{
    if (!mgr || !mgr->initialized) {
        return 0;
    }
    
    uint8_t count = 0;
    for (int i = 0; i < mgr->resource_count; i++) {
        if (mgr->resources[i].type == type) {
            count++;
        }
    }
    
    return count;
}

uint8_t resource_manager_get_total_count(const resource_manager_t* mgr)
{
    return mgr && mgr->initialized ? mgr->resource_count : 0;
}

bool resource_manager_has_capacity(const resource_manager_t* mgr)
{
    return mgr && mgr->initialized && 
           (mgr->resource_count < RESOURCE_MANAGER_MAX_RESOURCES);
}

// === DIAGNOSTICS ===

void resource_manager_get_stats(
    const resource_manager_t* mgr,
    resource_manager_stats_t* stats)
{
    if (!mgr || !stats) {
        return;
    }
    
    memset(stats, 0, sizeof(resource_manager_stats_t));
    
    if (!mgr->initialized) {
        return;
    }
    
    stats->active_resources = mgr->resource_count;
    stats->total_registered = mgr->total_resources_registered;
    stats->total_cleaned = mgr->total_resources_cleaned;
    stats->cleanup_failures = mgr->total_cleanup_failures;
    stats->emergency_cleanups = mgr->emergency_cleanups;
    stats->emergency_mode = mgr->emergency_mode;
}

void resource_manager_print_diagnostics(const resource_manager_t* mgr)
{
    if (!mgr || !mgr->initialized) {
        uart_write_string("Resource Manager: Not initialized\r\n");
        return;
    }
    
    uart_write_string("=== Resource Manager Diagnostics ===\r\n");
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Active resources: %d\r\n", mgr->resource_count);
    uart_write_string(msg);
    
    snprintf(msg, sizeof(msg), "Total registered: %lu\r\n", mgr->total_resources_registered);
    uart_write_string(msg);
    
    snprintf(msg, sizeof(msg), "Total cleaned: %lu\r\n", mgr->total_resources_cleaned);
    uart_write_string(msg);
    
    if (mgr->emergency_mode) {
        uart_write_string("Status: EMERGENCY MODE\r\n");
    }
    
    // List active resources
    for (int i = 0; i < mgr->resource_count; i++) {
        const resource_entry_t* entry = &mgr->resources[i];
        snprintf(msg, sizeof(msg), "Resource %d: %s (type=%d, state=%d)\r\n", 
                i, entry->resource_name ? entry->resource_name : "unnamed",
                entry->type, entry->state);
        uart_write_string(msg);
    }
}

bool resource_manager_validate(const resource_manager_t* mgr)
{
    if (!mgr || !mgr->initialized) {
        return false;
    }
    
    // Check for invalid resource states
    for (int i = 0; i < mgr->resource_count; i++) {
        const resource_entry_t* entry = &mgr->resources[i];
        
        if (entry->type == RESOURCE_TYPE_NONE || !entry->cleanup_fn) {
            return false; // Invalid resource entry
        }
    }
    
    return true;
}

// === STANDARD RESOURCE CLEANUP FUNCTIONS ===

void resource_cleanup_uart(void* uart_handle)
{
    // Reset UART to safe state
    uart_begin(115200); // Reset to default baud rate
}

void resource_cleanup_flash_context(void* flash_context)
{
    // Flash context cleanup would go here
    // For now, just mark as cleaned
}

void resource_cleanup_protocol_buffer(void* buffer)
{
    // Protocol buffer cleanup
    if (buffer) {
        // In real implementation, would free allocated buffer
        // For embedded system, might just mark as unused
    }
}

void resource_cleanup_staging_buffer(void* staging_buffer)
{
    // Staging buffer cleanup
    if (staging_buffer) {
        // Clear sensitive data from staging buffer
        memset(staging_buffer, 0, 8); // Assume 8-byte staging buffer
    }
}