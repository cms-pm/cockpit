/*
 * Embedded Hypervisor MVP - Main Entry Point
 * Phase 1, Chunk 1.2: VM Core Stack Operations
 */

#include <stdint.h>
#include "../lib/vm_core/vm_core.h"
#include "../lib/semihosting/semihosting.h"

// Test function (defined in test_vm_core.c)
int run_vm_tests(void);

// Vector table for ARM Cortex-M4
extern uint32_t _stack_start;

// Forward declarations
void Reset_Handler(void);
void Default_Handler(void);
void startup_init(void);

// Vector table
__attribute__((section(".vectors")))
const uint32_t vector_table[] = {
    (uint32_t)&_stack_start,    // Initial stack pointer
    (uint32_t)Reset_Handler,    // Reset handler
    (uint32_t)Default_Handler,  // NMI handler
    (uint32_t)Default_Handler,  // Hard fault handler
    // ... additional vectors would go here for full implementation
};

// Basic startup initialization
void startup_init(void)
{
    // Initialize data section (copy from Flash to RAM)
    extern uint32_t _data_start, _data_end, _data_load;
    uint32_t *src = &_data_load;
    uint32_t *dst = &_data_start;
    
    while (dst < &_data_end) {
        *dst++ = *src++;
    }
    
    // Initialize BSS section (zero)
    extern uint32_t _bss_start, _bss_end;
    dst = &_bss_start;
    while (dst < &_bss_end) {
        *dst++ = 0;
    }
}

// Reset handler - entry point after startup  
void Reset_Handler(void)
{
    // Initialize memory sections
    startup_init();
    
    debug_print("Embedded Hypervisor MVP Starting...");
    debug_print("Phase 1, Chunk 1.3: QEMU Integration");
    
    // Run VM core tests
    int test_result = run_vm_tests();
    
    // Report final result and exit
    if (test_result == 0) {
        debug_print("=== HYPERVISOR TESTS SUCCESSFUL ===");
        semihost_exit(0);  // Exit with success code
    } else {
        debug_print("=== HYPERVISOR TESTS FAILED ===");
        debug_print_dec("Failed test count", test_result);
        semihost_exit(1);  // Exit with failure code
    }
}

// Default handler for unimplemented interrupts
void Default_Handler(void)
{
    while (1) {
        // Hang on unexpected interrupt
    }
}