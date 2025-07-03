/*
 * Embedded Hypervisor MVP - Main Entry Point
 * Phase 1, Chunk 1.1: Project Structure Setup
 */

#include <stdint.h>

// Vector table for ARM Cortex-M4
extern uint32_t _stack_start;

// Forward declarations
void Reset_Handler(void);
void Default_Handler(void);

// Vector table
__attribute__((section(".vectors")))
const uint32_t vector_table[] = {
    (uint32_t)&_stack_start,    // Initial stack pointer
    (uint32_t)Reset_Handler,    // Reset handler
    (uint32_t)Default_Handler,  // NMI handler
    (uint32_t)Default_Handler,  // Hard fault handler
    // ... additional vectors would go here for full implementation
};

// Reset handler - entry point after startup
void Reset_Handler(void)
{
    // TODO: Initialize data/bss sections
    // TODO: Initialize VM core
    // TODO: Start hypervisor
    
    // For now, just infinite loop to verify QEMU launches
    while (1) {
        // Placeholder for VM execution loop
    }
}

// Default handler for unimplemented interrupts
void Default_Handler(void)
{
    while (1) {
        // Hang on unexpected interrupt
    }
}