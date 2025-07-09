/*
 * Simple Debug Test for Runtime Validator
 * Testing the most basic functionality
 */

#include "lib/component_vm/include/component_vm_c.h"
#include "lib/semihosting/semihosting.h"

int main(void) {
    semihost_write_string("=== Simple Debug Test ===\n");
    
    // Create VM instance
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        semihost_write_string("ERROR: Failed to create VM\n");
        return 1;
    }
    
    // Test basic PUSH/HALT program
    vm_instruction_c_t simple_program[] = {
        {0x01, 0, 42},  // OP_PUSH 42
        {0x00, 0, 0}    // OP_HALT
    };
    
    semihost_write_string("Testing basic PUSH/HALT...\n");
    bool result = component_vm_execute_program(vm, simple_program, 2);
    
    if (result) {
        semihost_write_string("SUCCESS: Basic program executed\n");
        if (component_vm_is_halted(vm)) {
            semihost_write_string("SUCCESS: VM halted properly\n");
        } else {
            semihost_write_string("ERROR: VM not halted\n");
        }
    } else {
        semihost_write_string("ERROR: Basic program failed\n");
        vm_c_error_t error = component_vm_get_last_error(vm);
        semihost_write_string("Error: ");
        semihost_write_string(component_vm_get_error_string(error));
        semihost_write_string("\n");
    }
    
    // Test memory operations
    semihost_write_string("Testing memory operations...\n");
    component_vm_reset(vm);
    
    vm_instruction_c_t memory_program[] = {
        {0x01, 0, 100},  // OP_PUSH 100
        {0x51, 0, 5},    // OP_STORE_GLOBAL 5
        {0x50, 0, 5},    // OP_LOAD_GLOBAL 5
        {0x00, 0, 0}     // OP_HALT
    };
    
    result = component_vm_execute_program(vm, memory_program, 4);
    
    if (result) {
        semihost_write_string("SUCCESS: Memory operations executed\n");
    } else {
        semihost_write_string("ERROR: Memory operations failed\n");
        vm_c_error_t error = component_vm_get_last_error(vm);
        semihost_write_string("Error: ");
        semihost_write_string(component_vm_get_error_string(error));
        semihost_write_string("\n");
    }
    
    component_vm_destroy(vm);
    semihost_write_string("=== Debug test complete ===\n");
    return 0;
}