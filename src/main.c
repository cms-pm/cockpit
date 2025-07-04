/*
 * Embedded Hypervisor MVP - Main Entry Point
 * Phase 1, Chunk 1.2: VM Core Stack Operations
 */

#include <stdint.h>
#include "../lib/vm_core/vm_core.h"
#include "../lib/semihosting/semihosting.h"

// Test functions
int run_vm_tests(void);           // VM core tests
int run_qemu_gpio_tests(void);    // QEMU-compatible GPIO tests
int run_button_tests(void);       // Button input tests
int run_arduino_function_tests(void); // Arduino function tests (Phase 2.3)

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
    debug_print("Phase 2, Chunk 2.3: Arduino Function Integration");
    
    // Run VM core tests
    int vm_test_result = run_vm_tests();
    
    // Run QEMU-compatible GPIO tests  
    int gpio_test_result = run_qemu_gpio_tests();
    
    // Run button input tests
    int button_test_result = run_button_tests();
    
    // Run Arduino function tests (Phase 2.3)
    int arduino_test_result = run_arduino_function_tests();
    
    // Combined test result
    int total_failures = vm_test_result + gpio_test_result + button_test_result + arduino_test_result;
    
    // Report final result and exit
    if (total_failures == 0) {
        debug_print("=== ALL HYPERVISOR TESTS SUCCESSFUL ===");
        debug_print("VM Core + GPIO + Button + Arduino Function tests passed");
        semihost_exit(0);  // Exit with success code
    } else {
        debug_print("=== SOME HYPERVISOR TESTS FAILED ===");
        debug_print_dec("VM test failures", vm_test_result);
        debug_print_dec("GPIO test failures", gpio_test_result);
        debug_print_dec("Button test failures", button_test_result);
        debug_print_dec("Arduino function test failures", arduino_test_result);
        debug_print_dec("Total failures", total_failures);
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