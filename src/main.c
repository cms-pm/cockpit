/*
 * ComponentVM Main Entry Point
 * Phase 3: Clean entry point for C++ ComponentVM with C wrapper
 * Following architectural cleanliness principles from CLAUDE.md
 */

#include <stdint.h>
#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"

// Forward declarations for test functions
int run_component_vm_tests(void);           // ComponentVM wrapper tests
int run_vm_core_tests(void);               // Migrated Phase 1 tests
int run_arduino_function_tests(void);      // Migrated Phase 2 tests  
int run_integration_tests(void);           // Migrated Phase 3 tests

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
    
    debug_print("ComponentVM Embedded Hypervisor Starting...");
    debug_print("Phase 3: C++ ComponentVM with C Wrapper Integration");
    
    // Track test results
    int total_failures = 0;
    
    // Run ComponentVM wrapper tests (basic functionality)
    debug_print("Running ComponentVM C Wrapper Tests...");
    int wrapper_test_result = run_component_vm_tests();
    total_failures += wrapper_test_result;
    
    // Run migrated Phase 1 tests (VM core functionality)
    debug_print("Running Phase 1: VM Core Tests...");
    int vm_core_result = run_vm_core_tests();
    total_failures += vm_core_result;
    
    // Run migrated Phase 2 tests (Arduino integration)
    debug_print("Running Phase 2: Arduino Integration Tests...");
    int arduino_result = run_arduino_function_tests();
    total_failures += arduino_result;
    
    // Run migrated Phase 3 tests (advanced features)
    debug_print("Running Phase 3: Integration Tests...");
    int integration_result = run_integration_tests();
    
    // Test minimal debug program to verify CALL fix
    debug_print("Testing minimal debug program (CALL fix validation)...");
    
    vm_instruction_c_t minimal_program[] = {
        {0x08, 0, 2},   // CALL setup (address 2)  
        {0x00, 0, 0},   // HALT
        {0x01, 0, 42},  // PUSH 42
        {0x51, 0, 9},   // STORE_GLOBAL global_var (index 9)
        {0x09, 0, 0}    // RET
    };
    
    ComponentVM_C* test_vm = component_vm_create();
    bool minimal_result = false;
    if (test_vm) {
        minimal_result = component_vm_execute_program(test_vm, minimal_program, 5);
        if (minimal_result && component_vm_is_halted(test_vm)) {
            debug_print("✓ Minimal debug program: PASS (CALL/RET working)");
        } else {
            debug_print("✗ Minimal debug program: FAIL");
            minimal_result = false;
        }
        component_vm_destroy(test_vm);
    }
    
    // Test no printf program to isolate printf issues
    debug_print("Testing no printf program (isolate printf hanging)...");
    
    vm_instruction_c_t no_printf_program[] = {
        {0x08, 0, 2},   // CALL setup (address 2)
        {0x00, 0, 0},   // HALT
        {0x01, 0, 123}, // PUSH 123
        {0x51, 0, 9},   // STORE_GLOBAL result (index 9)
        {0x09, 0, 0}    // RET
    };
    
    ComponentVM_C* no_printf_vm = component_vm_create();
    bool no_printf_result = false;
    if (no_printf_vm) {
        no_printf_result = component_vm_execute_program(no_printf_vm, no_printf_program, 5);
        if (no_printf_result && component_vm_is_halted(no_printf_vm)) {
            debug_print("✓ No printf program: PASS (basic function calls working)");
        } else {
            debug_print("✗ No printf program: FAIL");
            no_printf_result = false;
        }
        component_vm_destroy(no_printf_vm);
    }
    total_failures += integration_result;
    if (!minimal_result) total_failures++;
    if (!no_printf_result) total_failures++;
    
    // Report final result and exit
    if (total_failures == 0) {
        debug_print("=== ALL COMPONENTVM TESTS SUCCESSFUL ===");
        debug_print("✓ C++ ComponentVM architecture working");
        debug_print("✓ C wrapper interface validated");
        debug_print("✓ Mixed C/C++ compilation successful");
        debug_print("✓ 32-bit instruction format operational");
        debug_print("✓ Phase 1-3 feature migration complete");
        semihost_exit(0);  // Exit with success code
    } else {
        debug_print("=== COMPONENTVM TESTS FAILED ===");
        debug_print_dec("ComponentVM wrapper failures", wrapper_test_result);
        debug_print_dec("VM core test failures", vm_core_result);
        debug_print_dec("Arduino integration failures", arduino_result);
        debug_print_dec("Integration test failures", integration_result);
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
