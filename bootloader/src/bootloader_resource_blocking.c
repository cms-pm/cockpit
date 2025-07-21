/*
 * Bootloader Simplified Resource Management Implementation
 * 
 * Simple resource tracking for blocking operations.
 * Prevents resource leaks and hardware lockups.
 */

#include "bootloader_resource_blocking.h"
#include "host_interface/host_interface.h"
#include <string.h>

// Global resource manager instance
static bootloader_resource_manager_blocking_t g_resource_manager = {0};

// Resource name strings
static const char* resource_names[RESOURCE_COUNT] = {
    "UART",
    "FLASH", 
    "CRITICAL_SECTION"
};

void bootloader_resource_manager_init_blocking(bootloader_resource_manager_blocking_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(bootloader_resource_manager_blocking_t));
    
    // Initialize resource states
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        manager->resources[i].allocated = false;
        manager->resources[i].allocation_time = 0;
        manager->resources[i].cleanup_function = NULL;
        manager->resources[i].resource_name = resource_names[i];
    }
    
    manager->initialized = true;
}

bool bootloader_resource_allocate_blocking(bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type, void (*cleanup_fn)(void)) {
    if (!manager || !manager->initialized || type >= RESOURCE_COUNT) {
        return false;
    }
    
    // Check if already allocated
    if (manager->resources[type].allocated) {
        return false;  // Resource already in use
    }
    
    // Allocate resource
    manager->resources[type].allocated = true;
    manager->resources[type].allocation_time = get_tick_ms();
    manager->resources[type].cleanup_function = cleanup_fn;
    manager->total_allocations++;
    
    return true;
}

void bootloader_resource_deallocate_blocking(bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type) {
    if (!manager || !manager->initialized || type >= RESOURCE_COUNT) {
        return;
    }
    
    if (manager->resources[type].allocated) {
        // Call cleanup function if provided
        if (manager->resources[type].cleanup_function) {
            manager->resources[type].cleanup_function();
        }
        
        // Deallocate resource
        manager->resources[type].allocated = false;
        manager->resources[type].allocation_time = 0;
        manager->resources[type].cleanup_function = NULL;
        manager->total_cleanups++;
    }
}

void bootloader_resource_cleanup_all_blocking(bootloader_resource_manager_blocking_t* manager) {
    if (!manager || !manager->initialized) {
        return;
    }
    
    // Cleanup all allocated resources in reverse order
    for (int i = RESOURCE_COUNT - 1; i >= 0; i--) {
        if (manager->resources[i].allocated) {
            bootloader_resource_deallocate_blocking(manager, (bootloader_resource_type_t)i);
        }
    }
}

bool bootloader_resource_is_allocated_blocking(const bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type) {
    if (!manager || !manager->initialized || type >= RESOURCE_COUNT) {
        return false;
    }
    
    return manager->resources[type].allocated;
}

uint32_t bootloader_resource_get_allocation_count_blocking(const bootloader_resource_manager_blocking_t* manager) {
    if (!manager || !manager->initialized) {
        return 0;
    }
    
    uint32_t count = 0;
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        if (manager->resources[i].allocated) {
            count++;
        }
    }
    
    return count;
}

const char* bootloader_resource_get_name_blocking(bootloader_resource_type_t type) {
    if (type >= RESOURCE_COUNT) {
        return "UNKNOWN";
    }
    
    return resource_names[type];
}

// Specific resource cleanup functions
void cleanup_uart_resources(void) {
    // UART cleanup is minimal for blocking implementation
    // Host Interface UART operations are inherently safe
    // No DMA or interrupts to disable
}

void cleanup_flash_resources(void) {
    // Ensure flash is locked after operations
    // This would call platform-specific flash lock functions
    // For now, placeholder implementation
}

void cleanup_critical_section_resources(void) {
    // Re-enable interrupts if they were disabled
    // For blocking implementation, critical sections are minimal
    __enable_irq();
}

// Global resource manager access
bootloader_resource_manager_blocking_t* bootloader_get_resource_manager_blocking(void) {
    return &g_resource_manager;
}