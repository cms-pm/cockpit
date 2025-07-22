/*
 * Build Validation Test - vm_bootloader Library
 * 
 * Simple test to verify the vm_bootloader library compiles correctly
 * and basic API functions are accessible.
 */

#ifdef TESTING

#include "vm_bootloader.h"
#include <stddef.h>

// Test that basic API functions can be called
int test_vm_bootloader_build_validation(void)
{
    // Test context allocation (stack-based)
    vm_bootloader_context_t test_context;
    
    // Test default configuration
    vm_bootloader_config_t test_config;
    vm_bootloader_get_default_config(&test_config);
    
    // Test initialization (should succeed)
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&test_context, &test_config);
    
    // Test basic query functions
    bool is_init = vm_bootloader_is_initialized(&test_context);
    bool is_ready = vm_bootloader_is_ready(&test_context);
    vm_bootloader_state_t state = vm_bootloader_get_current_state(&test_context);
    
    // Test cleanup
    vm_bootloader_cleanup(&test_context);
    
    // Suppress unused variable warnings
    (void)init_result;
    (void)is_init;
    (void)is_ready;
    (void)state;
    
    return 0; // Success
}

#endif // TESTING