/*
 * Simple memory protection validation test
 * Tests that memory protection canaries are initialized properly
 */

#include "../lib/vm_core/vm_core.h"

void test_memory_protection_simple() {
    // Simple test to verify memory protection functions compile and work
    vm_state_t vm;
    vm_error_t result;
    
    // Initialize VM (this should set up memory protection)
    result = vm_init(&vm);
    if (result != VM_OK) {
        debug_print_dec("VM init failed", result);
        return;
    }
    
    // Check canaries
    result = vm_check_stack_canaries(&vm);
    if (result == VM_OK) {
        debug_print_dec("Stack canaries OK", 1);
    } else {
        debug_print_dec("Stack canaries FAILED", result);
    }
    
    result = vm_check_heap_guards(&vm);
    if (result == VM_OK) {
        debug_print_dec("Heap guards OK", 1);
    } else {
        debug_print_dec("Heap guards FAILED", result);
    }
    
    // Test normal stack operations
    result = vm_push(&vm, 42);
    if (result == VM_OK) {
        uint32_t value;
        result = vm_pop(&vm, &value);
        if (result == VM_OK && value == 42) {
            debug_print_dec("Stack operations OK", value);
        } else {
            debug_print_dec("Stack operations FAILED", result);
        }
    } else {
        debug_print_dec("Push failed", result);
    }
}