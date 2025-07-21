/*
 * Bootloader Simplified Resource Management
 * 
 * Simple resource tracking for blocking operations.
 * Balanced granularity - tracks major resources with cleanup functions.
 */

#ifndef BOOTLOADER_RESOURCE_BLOCKING_H
#define BOOTLOADER_RESOURCE_BLOCKING_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Resource types for blocking bootloader
typedef enum {
    RESOURCE_UART = 0,
    RESOURCE_FLASH,
    RESOURCE_CRITICAL_SECTION,
    RESOURCE_COUNT
} bootloader_resource_type_t;

// Resource state tracking
typedef struct {
    bool allocated;
    uint32_t allocation_time;
    void (*cleanup_function)(void);
    const char* resource_name;
} resource_state_t;

// Simple resource manager for blocking operations
typedef struct {
    resource_state_t resources[RESOURCE_COUNT];
    uint32_t total_allocations;
    uint32_t total_cleanups;
    bool initialized;
} bootloader_resource_manager_blocking_t;

// Resource management functions
void bootloader_resource_manager_init_blocking(bootloader_resource_manager_blocking_t* manager);
bool bootloader_resource_allocate_blocking(bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type, void (*cleanup_fn)(void));
void bootloader_resource_deallocate_blocking(bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type);
void bootloader_resource_cleanup_all_blocking(bootloader_resource_manager_blocking_t* manager);

// Resource query functions
bool bootloader_resource_is_allocated_blocking(const bootloader_resource_manager_blocking_t* manager, bootloader_resource_type_t type);
uint32_t bootloader_resource_get_allocation_count_blocking(const bootloader_resource_manager_blocking_t* manager);
const char* bootloader_resource_get_name_blocking(bootloader_resource_type_t type);

// Specific resource cleanup functions
void cleanup_uart_resources(void);
void cleanup_flash_resources(void);
void cleanup_critical_section_resources(void);

// Global resource manager access
bootloader_resource_manager_blocking_t* bootloader_get_resource_manager_blocking(void);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_RESOURCE_BLOCKING_H